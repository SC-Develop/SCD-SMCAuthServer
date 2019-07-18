#ifndef SCD_PCSC_H
#define SCD_PCSC_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <wintypes.h>
#include <winscard.h>

class SCD_PCSC
{
   public:

     struct card_data
     {
        int freeError = 0;  // error on free resource
        int error     = 0;  // card error
        int atrvalid  = 0;  // set to 1 if readed atrcode is valid

        char *errmsg     = NULL; // card error message
        char *freeErrMsg = NULL; // free resource error message

        unsigned char *data   = NULL; // readed data buffer
        unsigned int  datalen = 0;    // data len (bytes)
     };

   private:

     BYTE pbAtr[MAX_ATR_SIZE];

     char pbReader[MAX_READERNAME];

     char msg[1024];       // card message

     char lastError[1024]; // last error message

     char *ptr;
     char **readers = NULL; // array of readers;

     int nbReaders; // readers number
     int reader_nb;

     LPSTR mszReaders = NULL;

     SCARDHANDLE  hCard;    // card handle
     SCARDCONTEXT hContext; // current card context

     DWORD dwActiveProtocol;
     DWORD dwReaderLen;
     DWORD dwState;
     DWORD dwProt;
     DWORD dwAtrLen;
     DWORD dwReaders;

     void init();

     int freeResource(card_data *card);

   public:

     SCD_PCSC();

     struct card_data CheckCard();
};

#endif // SCD_PCSC_H
