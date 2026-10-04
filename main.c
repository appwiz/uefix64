/*
 * Minimal UEFI application for x64 and AArch64 that prints "welcome".
 *
 * Self-contained: defines only the slice of the UEFI spec it needs, so no
 * gnu-efi or EDK II headers are required.
 */

#include <stdint.h>

typedef uint64_t EFI_STATUS;
typedef void *EFI_HANDLE;
typedef void *EFI_EVENT;
typedef uint16_t CHAR16;
typedef uint64_t UINTN;

#define EFI_SUCCESS 0
/* x64 UEFI uses the Microsoft calling convention; AArch64 uses the standard one. */
#if defined(__x86_64__)
#define EFIAPI __attribute__((ms_abi))
#else
#define EFIAPI
#endif

typedef struct {
    uint64_t Signature;
    uint32_t Revision;
    uint32_t HeaderSize;
    uint32_t CRC32;
    uint32_t Reserved;
} EFI_TABLE_HEADER;

/* Simple Text Output Protocol */

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS(EFIAPI *EFI_TEXT_RESET)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
                                           uint8_t ExtendedVerification);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
                                            CHAR16 *String);

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET Reset;
    EFI_TEXT_STRING OutputString;
    /* Remaining members are unused here. */
};

/* Simple Text Input Protocol */

typedef struct {
    uint16_t ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef EFI_STATUS(EFIAPI *EFI_INPUT_RESET)(EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
                                            uint8_t ExtendedVerification);
typedef EFI_STATUS(EFIAPI *EFI_INPUT_READ_KEY)(EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
                                               EFI_INPUT_KEY *Key);

struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    EFI_EVENT WaitForKey;
};

/* Boot Services */

typedef EFI_STATUS(EFIAPI *EFI_WAIT_FOR_EVENT)(UINTN NumberOfEvents, EFI_EVENT *Event,
                                               UINTN *Index);
typedef EFI_STATUS(EFIAPI *EFI_SET_WATCHDOG_TIMER)(UINTN Timeout, uint64_t WatchdogCode,
                                                   UINTN DataSize, CHAR16 *WatchdogData);

typedef struct {
    EFI_TABLE_HEADER Hdr;
    void *Unused1[9];  /* RaiseTPL .. SetTimer */
    EFI_WAIT_FOR_EVENT WaitForEvent;
    void *Unused2[19]; /* SignalEvent .. Stall */
    EFI_SET_WATCHDOG_TIMER SetWatchdogTimer;
    /* Remaining members are unused here. */
} EFI_BOOT_SERVICES;

/* System Table */

typedef struct {
    EFI_TABLE_HEADER Hdr;
    CHAR16 *FirmwareVendor;
    uint32_t FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    void *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
    /* Remaining members are unused here. */
} EFI_SYSTEM_TABLE;

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *in = SystemTable->ConIn;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *out = SystemTable->ConOut;
    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;
    EFI_INPUT_KEY key;
    UINTN index;

    (void)ImageHandle;

    /* The firmware resets the machine if an app runs longer than 5 minutes. */
    bs->SetWatchdogTimer(0, 0, 0, 0);

    out->OutputString(out, u"welcome\r\n");
    out->OutputString(out, u"Press any key to exit.\r\n");

    /*
     * Wait for a key before returning; otherwise the firmware moves on to
     * its next boot option (the setup menu) and clears "welcome" off the
     * screen straight away.
     */
    in->Reset(in, 0);
    bs->WaitForEvent(1, &in->WaitForKey, &index);
    in->ReadKeyStroke(in, &key);

    return EFI_SUCCESS;
}
