#include <stdio.h>
#include <string.h>
#include <windows.h>

#ifdef _MSC_VER
#pragma warning( disable : 4996 )
#endif

HANDLE com4()
{
    HANDLE hSerial;
    DCB dcb;
    COMMTIMEOUTS timeouts;

    hSerial = CreateFile("COM4",
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         0,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         0);
    if (hSerial == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    FillMemory(&dcb, sizeof(dcb), 0);
    if (!GetCommState(hSerial, &dcb)) {
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    dcb.BaudRate = CBR_115200;
    if (!SetCommState(hSerial, &dcb)) {
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 100;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return hSerial;
}

void write(HANDLE hSerial, const char cmd[])
{
    char cmd2[64];
    DWORD bytesWritten;

    strcpy(cmd2, cmd);
    strcat(cmd2, "\r");

    WriteFile(hSerial, cmd2, strlen(cmd2), &bytesWritten, 0);

    OutputDebugString("w:");
    OutputDebugString(cmd);
    OutputDebugString("\n");
}

const char *read(HANDLE hSerial)
{
    char *ret, recv[64];
    DWORD bytesRead = 0;

    ReadFile(hSerial, recv, sizeof(recv)-1, &bytesRead, 0);

    recv[bytesRead] = 0;
    ret = recv;
    if (bytesRead > 0) {
        // cleanup response
        while ((bytesRead > 0) && (recv[bytesRead-1] == '\r' || recv[bytesRead-1] == '\n'))
            recv[--bytesRead] = 0;
        while (*ret == '\r' || *ret == '\n')
            ret++;
    }

    OutputDebugString("r:");
    OutputDebugString(*ret ? ret : "<empty>");
    OutputDebugString("\n");

    return ret;
}

const char *sendAndReceive(HANDLE hSerial, const char cmd[])
{
    write(hSerial, cmd);
    return read(hSerial);
}

const char *sendAndReceiveTimeout(HANDLE hSerial, const char cmd[], int timeout)
{
    int t;
    write(hSerial, cmd);
    for (t = 0; t < timeout; t++) {
        const char *ret = read(hSerial);
        if (*ret != '\0')
            return ret;
        Sleep(1000);
    }
    return "";
}

int setupGSM(HANDLE hSerial)
{
    int tries;

    // Do we have communication?
    if ((hSerial == INVALID_HANDLE_VALUE) || (strcmp("OK", sendAndReceive(hSerial, "AT")) != 0))
        return 0;
    /*
        for (tries = 0; tries < 10; tries++) {
            const char *recv = sendAndReceiveTimeout(hSerial, "AT+COPS=?", 30);
            if (*recv != '\0')
                break;
        }
    */
    return 1;
}

void sendSMS(HANDLE hSerial, const char msg[])
{
    const char ctrlz[2] = { 26, 0 };
    sendAndReceive(hSerial, "AT+CMGS=\"0709124262\"");
    sendAndReceive(hSerial, msg);
    sendAndReceive(hSerial, ctrlz);
}

int main()
{
    HANDLE hSerial;

    hSerial = com4();
    setupGSM(hSerial);

    sendSMS(hSerial, "testar");

    return 0;
}
