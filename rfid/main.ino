#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define RED 2
#define Green 3

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
/* Create an instance of MIFARE_Key */
// MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

// Number of known default keys (hard-coded)
// NOTE: Synchronize the NR_KNOWN_KEYS define with the defaultKeys[] array
#define NR_KNOWN_KEYS 8
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] = {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
    {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
    {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
    {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
    {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
    {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
    {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};

byte prugio_data[] = {
    0x19, 0x24, 0x47, 0x7E, 0x04, 0x08, 0x04, 0x00, 0x02, 0x5D, 0x53, 0xAF, 0x02, 0x2F, 0x82, 0x1D};

byte buffer_len = 18;
byte read_buffer[18];

void setup()
{
    Serial.begin(9600); // Initiate a serial communication
    SPI.begin();        // Initiate  SPI bus
    mfrc522.PCD_Init(); // Initiate MFRC522
    Serial.println("Approximate your card to the reader...");
    Serial.println();
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
    digitalWrite(Green, LOW);
    digitalWrite(RED, LOW);
}

void loop()
{
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++)
    {
        key.keyByte[i] = knownKeys[0][i];
    }
    write_data(0, key, prugio_data);

    // Try the known default keys
    // MFRC522::MIFARE_Key key;
    // for (byte k = 0; k < NR_KNOWN_KEYS; k++)
    // {
    //     // Copy the known key into the MIFARE_Key structure
    //     for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++)
    //     {
    //         key.keyByte[i] = knownKeys[k][i];
    //     }
    //     // Try the key
    //     // if (try_key(0, &key))
    //     // {
    //     //     // Found and reported on the key and block,
    //     //     // no need to try other keys for this PICC
    //     //     break;
    //     // }

    //     read_block(0, &key, read_buffer);
    //     // read_block(1, &key, read_buffer);
    //     // read_block(2, &key, read_buffer);

    //     // http://arduino.stackexchange.com/a/14316
    //     if (!mfrc522.PICC_IsNewCardPresent())
    //         break;
    //     if (!mfrc522.PICC_ReadCardSerial())
    //         break;
    // }
}

bool read_block(byte block, MFRC522::MIFARE_Key *key, byte data[])
{
    bool result = false;
    byte buffer[18];
    MFRC522::StatusCode status;

    // Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Read block
    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    else
    {
        // Successful read
        result = true;
        Serial.print(F("Success with key:"));
        dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
        Serial.println();
        // Dump block data
        Serial.print(F("Block "));
        Serial.print(block);
        Serial.print(F(":"));
        dump_byte_array(buffer, 16);
        Serial.println();
    }
    Serial.println();

    mfrc522.PICC_HaltA();      // Halt PICC
    mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD

    return result;
}

/*
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize)
{
    for (byte i = 0; i < bufferSize; i++)
    {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
        // Serial.print(" ");
        // Serial.print(buffer[i]);
    }
}

bool try_key(byte block, MFRC522::MIFARE_Key *key)
{
    bool result = false;
    byte buffer[18];
    MFRC522::StatusCode status;

    // Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        // Serial.print(F("PCD_Authenticate() failed: "));
        // Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Read block
    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK)
    {
        // Serial.print(F("MIFARE_Read() failed: "));
        // Serial.println(mfrc522.GetStatusCodeName(status));
    }
    else
    {
        // Successful read
        result = true;
        Serial.print(F("Success with key:"));
        dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
        Serial.println();
        // Dump block data
        Serial.print(F("Block "));
        Serial.print(block);
        Serial.print(F(":"));
        dump_byte_array(buffer, 16);
        Serial.println();
    }
    Serial.println();

    mfrc522.PICC_HaltA();      // Halt PICC
    mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
    return result;
}

void write_data(int blockNum, MFRC522::MIFARE_Key key, byte blockData[])
{
    /* Authenticating the desired data block for write access using Key A */
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Authentication failed for Write: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    else
    {
        Serial.println("Authentication success");
    }

    /* Write data to the block */
    status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Writing to Block failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    else
    {
        Serial.println("Data was written into Block successfully");
    }
}