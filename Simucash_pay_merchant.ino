/**************************************************************************/
/*!
    @file     mifareclassic_memdump.pde
    @author   Adafruit Industries
  @license  BSD (see license.txt)

    This example attempts to dump the contents of a Mifare Classic 1K card

    Note that you need the baud rate to be 115200 because we need to print
  out the data and read from the card at the same time!

    This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
    This library works with the Adafruit NFC breakout
      ----> https://www.adafruit.com/products/364

    Check out the links above for our tutorials and wiring diagrams
    These chips use SPI or I2C to communicate

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

*/
/**************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a SPI connection:
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
//Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void setup(void) {
  // has to be fast to dump the entire memory contents!
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero

  // Serial.println("Looking for PN532...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
//  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
//  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
//  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();

//  Serial.println("Waiting for an ISO14443A Card ...");
}


void loop(void) {
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t currentblock;                     // Counter to keep track of which block we're on
  bool authenticated = false;               // Flag to indicate if the sector is authenticated
  uint8_t data[16];                         // Array to store block data during reads
  int tAmount;                              // Transfer amount
  int amountBlock=4;                        // Memory block with card amount
  int terminalId = 18923;                   // ID of the terminal being paid
  String storedStr;                         // String of the stored amount
  int storedAmounti;                        // Amount stored on the card before payment
  int storedAmountf;                        // Amount stored on the card after payment
  uint8_t num[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  

  // Keyb on NDEF and Mifare Classic should be the same
  uint8_t keyuniversal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  // Get payment amount
  Serial.println("Enter the amount to charge your customer:");
  // Wait for use to input the amount to update with
  while (!Serial.available());
  // Amount was input
  tAmount = Serial.parseInt();
  Serial.print("Hand to customer to pay");
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    // Display some basic information about the card
//    Serial.println("Found an ISO14443A card");
//    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
//    Serial.print("  UID Value: ");
//    nfc.PrintHex(uid, uidLength);
//    Serial.println("");

    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ...
//      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");      
//      success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, amountBlock, 1, keyuniversal);
//      if (success){
//        authenticated = true;
//      }else{
//        Serial.println("Authentication error");
//      }
//      success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
//      if(success){
//        storedAmounti = String(data[14],HEX)+String(data[15],HEX);
//        Serial.println(storedAmounti);
//      }

      // Now we try to go through all 16 sectors (each having 4 blocks)
      // authenticating each sector, and then dumping the blocks
      currentblock=4;
      if (nfc.mifareclassic_IsFirstBlock(currentblock)) authenticated = false;

      // If the sector hasn't been authenticated, do so first
      if (!authenticated)
      {
        // Starting of a new sector ... try to to authenticate
//        Serial.print("------------------------Sector ");Serial.print(currentblock/4, DEC);Serial.println("-------------------------");
        if (currentblock == 0)
        {
            // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
            // or 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 for NDEF formatted cards using key a,
            // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
            success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, keyuniversal);
        }
        else
        {
            // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
            // or 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 for NDEF formatted cards using key a,
            // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
            success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, keyuniversal);
        }
        if (success)
        {
          authenticated = true;
        }
        else
        {
          Serial.println("Authentication error");
        }
      }
      // If we're still not authenticated just skip the block
      if (!authenticated)
      {
        Serial.print("Block ");Serial.print(currentblock, DEC);Serial.println(" unable to authenticate");
      }
      else
      {
        // Authenticated ... we should be able to read the block now
        // Dump the data into the 'data' array
        success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
        if (success)
        {
          // Read successful
//          Serial.print("Block ");Serial.print(currentblock, DEC);
//          if (currentblock < 10)
//          {
//            Serial.print("  ");
//          }
//          else
//          {
//            Serial.print(" ");
//          }
//          // Dump the raw data
//          nfc.PrintHex(data, 16);
          if((data[15]&0xff)<16){
            storedStr = String(data[14]&0xff,HEX)+String("0")+String(data[15]&0xff,HEX);
          }else{
            storedStr = String(data[14]&0xff,HEX)+String(data[15]&0xff,HEX);
          }
//          Serial.print("data[14]: ");Serial.println(data[14],HEX);
//          Serial.print("data[15]: ");Serial.println(String(data[15],HEX));
          char buff[32];
          storedStr.toCharArray(buff,32);
          storedAmounti = strtoul(buff,NULL,16);
//          Serial.print("storedStr: ")+Serial.println(storedStr);
//          Serial.print("storedAmounti: ")+Serial.println(storedAmounti,DEC); 
//          Serial.print("tAmount: ")+Serial.println(tAmount,DEC);

          // Check that there is enough stored value to make the payment
          // Serial.print(tAmount,DEC); Serial.println(storedAmounti,DEC);
          if (storedAmounti >= tAmount){
            // do transfer
            storedAmountf = storedAmounti - tAmount;
            num[15] = (byte)(storedAmountf & 0xff);
            num[14] = (byte)(storedAmountf >> 8);
//            if(storedAmountf<0x0100){
//              num[15] = (byte)(storedAmountf & 0xff);
//              num[14] = (byte)(storedAmountf >> 8);
//            }else{
//              num[15]=storedAmountf;
//            }
//            Serial.print("num[14]: ");Serial.println(String(num[14],HEX));
//            Serial.print("num[15]: ");Serial.println(String(num[15],HEX));
            success = nfc.mifareclassic_WriteDataBlock(4, num);
            Serial.print("Payment complete! You have: $")+Serial.println(storedAmountf,DEC);
          }else{
            Serial.println("Not enough funds to complete the transfer.");
          }
        }
        else
        {
          // Oops ... something happened
          Serial.print("Block ");Serial.print(currentblock, DEC);
          Serial.println(" unable to read this block");
        }
        // Authenticated ... we should be able to read the block now
        // Dump the data into the 'data' array
        success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
        if (success)
        {
          // Read successful
//          Serial.print("Block ");Serial.print(currentblock, DEC);
//          if (currentblock < 10)
//          {
//            Serial.print("  ");
//          }
//          else
//          {
//            Serial.print(" ");
//          }
//          // Dump the raw data
//          nfc.PrintHex(data, 16);
        }else{
          // Oops ... something happened
          Serial.print("Block ");Serial.print(currentblock, DEC);
          Serial.println(" unable to read this block");
        }
      }
    }
    else
    {
      Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
    }
  }
  // Wait a bit before trying again
  Serial.println("\nDone!\n");
  Serial.flush();
  while (!Serial.available());
  while (Serial.available()) {
  Serial.read();
  }
  Serial.flush();
}
