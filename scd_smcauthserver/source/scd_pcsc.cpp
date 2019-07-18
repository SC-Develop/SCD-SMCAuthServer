#include "scd_pcsc.h"

/**
 * @brief SCD_PCSC::SCD_PCSC
 */
SCD_PCSC::SCD_PCSC()
{

}

/**
 * @brief SCD_PCSC::init
 */
void SCD_PCSC::init()
{
   bzero(pbAtr,MAX_ATR_SIZE);
   bzero(pbReader,MAX_READERNAME);
   bzero(msg,1024);
   bzero(lastError,1024);
}

/**
 * @brief SCD_PCSC::freeResource free allocated resource and, on error, set lastError to error message
 * @return 0:error, 1:success;
 */
int SCD_PCSC::freeResource(SCD_PCSC::card_data *card)
{
   card->freeError = 0;

   if (mszReaders)
   {
      SCardFreeMemory(hContext, mszReaders); // free the null terminated readers string
   }

   LONG rv = SCardReleaseContext(hContext); // release card context

   if (rv != SCARD_S_SUCCESS)
   {
      sprintf(lastError, "ERROR: SCardReleaseContext: %s (0x%lX)\n", pcsc_stringify_error(rv), rv);

      card->freeError  = 1;
      card->freeErrMsg = lastError;
   }

   if (readers)
   {
      free(readers); // free readers table
   }

   return !card->freeError;
}

/**
 * @brief SCD_PCSC::CheckCard
 * @return
 */
SCD_PCSC::card_data SCD_PCSC::CheckCard()
{
   struct card_data card;

   card.atrvalid = 0;

   init();

   // Get SCard Context ----------------------------------------------------------------------

   LONG rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

   if (rv != SCARD_S_SUCCESS)
   {
      sprintf(msg,"ERROR: SCardEstablishContext: Cannot Connect to Resource Manager %lX\n", rv);

      card.errmsg = msg;
      card.error  = 1;

      freeResource(&card);

      return card;
   }

   // Get the available readers list ---------------------------------------------------------

   dwReaders = SCARD_AUTOALLOCATE;

   rv = SCardListReaders(hContext, NULL, (LPSTR)&mszReaders, &dwReaders);

   if (rv != SCARD_S_SUCCESS)
   {
     sprintf(msg,"ERROR: SCardListReaders: %s (0x%lX)\n", pcsc_stringify_error(rv), rv);

     card.errmsg = msg;
     card.error  = 1;

     freeResource(&card);

     return card;
   }

   // Extract readers from the null separated string and get the total number of readers -----

   nbReaders = 0;

   ptr = mszReaders; // get the null separated string of readers

   while (*ptr != '\0')
   {
      ptr += strlen(ptr)+1;
      nbReaders++;
   }

   if (nbReaders == 0)
   {
      sprintf(msg,"ERROR: No reader found\n");

      card.errmsg = msg;
      card.error  = 1;

      freeResource(&card);

      return card;
   }

   // allocate the readers table -------------------------------------------------------------

   readers = (char **) calloc(nbReaders, sizeof(char *)); // allocate readers table (array of string)

   if (NULL == readers)
   {
      sprintf(msg,"ERROR: Not enough memory for readers[]\n");

      card.errmsg = msg;
      card.error  = 1;

      freeResource(&card);

      return card;
   }

   // extract the readers from null separated string mszReaders, and set the readers table ---

   nbReaders = 0;

   ptr = mszReaders;

   while (*ptr != '\0')
   {
      readers[nbReaders] = ptr; // set reader table
      ptr += strlen(ptr)+1;
      nbReaders++;
   }

   reader_nb = 0;

   // connect to a card ----------------------------------------------------------------------

   dwActiveProtocol = -1;

   rv = SCardConnect(hContext, readers[reader_nb], SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);

   if (rv != SCARD_S_SUCCESS)
   {
      sprintf(msg,"ERROR: SCardConnect: %s (0x%lX)\n", pcsc_stringify_error(rv), rv);

      card.errmsg = msg;
      card.error  = 1;

      freeResource(&card);

      return card;
   }

   // get card status ------------------------------------------------------------------------

   dwAtrLen    = sizeof(pbAtr);
   dwReaderLen = sizeof(pbReader);

   rv = SCardStatus(hCard, pbReader, &dwReaderLen, &dwState, &dwProt, pbAtr, &dwAtrLen);

   if (rv != SCARD_S_SUCCESS)
   {
      sprintf(msg,"ERROR: SCardStatus: %s (0x%lX)\n", pcsc_stringify_error(rv), rv);

      card.errmsg = msg;
      card.error  = 1;

      freeResource(&card);

      return card;
   }
   else
   {
      card.data     = pbAtr;
      card.datalen  = dwAtrLen;
      card.error    = 0;
      card.atrvalid = 1;
   }

   // card disconnect ------------------------------------------------------------------------

   rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);

   if (rv != SCARD_S_SUCCESS)
   {
      sprintf(msg,"ERROR: SCardDisconnect: %s (0x%lX)\n", pcsc_stringify_error(rv), rv);

      card.errmsg = msg;
      card.error  = 1;
   }

   // free all resource ----------------------------------------------------------------------

   freeResource(&card);

   return card;
}
