/****************************************************************************************************************************
  FS_Test.ino - Filesystem wrapper for FS (LittleFS and FATFS) on the Mbed Nano-33-BLE
  
  For MBED nRF52840-based boards such as Nano_33_BLE, Nano_33_BLE_Sense.
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/FS_Nano33BLE
  Licensed under MIT license

  Version: 1.0.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      29/08/2021 Initial coding to support MBED nRF52840-based boards such as Nano_33_BLE, etc.
*****************************************************************************************************************************/

#define FS_NANO33BLE_VERSION_MIN_TARGET      "FS_Nano33BLE v1.1.0"
#define FS_NANO33BLE_VERSION_MIN             1001000

#define _FS_LOGLEVEL_               1
#define NANO33BLE_FS_SIZE_KB        256

#define FORCE_REFORMAT              false

// Default USING_LITTLEFS. Uncomment to not USING_LITTLEFS => USING_FATFS. 
//#define USING_LITTLEFS              false

#include <FS_Nano33BLE.h>

FileSystem_MBED *myFS;

void readCharsFromFile(const char * path) 
{
  Serial.print("readCharsFromFile: "); Serial.print(path);

  FILE *file = fopen(path, "r");
  
  if (file) 
  {
    Serial.println(" => Open OK");
  }
  else
  {
    Serial.println(" => Open Failed");
    return;
  }

  char c;

  while (true) 
  {
    c = fgetc(file);
    
    if ( feof(file) ) 
    { 
      break;
    }
    else   
      Serial.print(c);
  }
   
  fclose(file);
}

void readFile(const char * path) 
{
  Serial.print("Reading file: "); Serial.print(path);

  FILE *file = fopen(path, "r");
  
  if (file) 
  {
    Serial.println(" => Open OK");
  }
  else
  {
    Serial.println(" => Open Failed");
    return;
  }

  char c;
  uint32_t numRead = 1;
  
  while (numRead) 
  {
    numRead = fread((uint8_t *) &c, sizeof(c), 1, file);

    if (numRead)
      Serial.print(c);
  }
  
  fclose(file);
}

void writeFile(const char * path, const char * message, size_t messageSize) 
{
  Serial.print("Writing file: "); Serial.print(path);

  FILE *file = fopen(path, "w");
  
  if (file) 
  {
    Serial.println(" => Open OK");
  }
  else
  {
    Serial.println(" => Open Failed");
    return;
  }
 
  if (fwrite((uint8_t *) message, 1, messageSize, file)) 
  {
    Serial.println("* Writing OK");
  } 
  else 
  {
    Serial.println("* Writing failed");
  }
  
  fclose(file);
}

void appendFile(const char * path, const char * message, size_t messageSize) 
{
  Serial.print("Appending file: "); Serial.print(path);

  FILE *file = fopen(path, "a");
  
  if (file) 
  {
    Serial.println(" => Open OK");
  }
  else
  {
    Serial.println(" => Open Failed");
    return;
  }

  if (fwrite((uint8_t *) message, 1, messageSize, file)) 
  {
    Serial.println("* Appending OK");
  } 
  else 
  {
    Serial.println("* Appending failed");
  }
   
  fclose(file);
}

void deleteFile(const char * path) 
{
  Serial.print("Deleting file: "); Serial.print(path);
  
  if (remove(path) == 0) 
  {
    Serial.println(" => OK");
  }
  else
  {
    Serial.println(" => Failed");
    return;
  }
}

void renameFile(const char * path1, const char * path2) 
{
  Serial.print("Renaming file: "); Serial.print(path1);
  Serial.print(" to: "); Serial.print(path2);
  
  if (rename(path1, path2) == 0) 
  {
    Serial.println(" => OK");
  }
  else
  {
    Serial.println(" => Failed");
    return;
  }
}

void testFileIO(const char * path) 
{
  Serial.print("Testing file I/O with: "); Serial.print(path);

  #define BUFF_SIZE     512
  
  static uint8_t buf[BUFF_SIZE];
  
  FILE *file = fopen(path, "w");
  
  if (file) 
  {
    Serial.println(" => Open OK");
  }
  else
  {
    Serial.println(" => Open Failed");
    return;
  }

  size_t i;
  Serial.println("- writing" );
  
  uint32_t start = millis();

  size_t result = 0;

  // Write a file only 1/4 of NANO33BLE_FS_SIZE_KB
  for (i = 0; i < NANO33BLE_FS_SIZE_KB / 2; i++) 
  {
    result = fwrite(buf, BUFF_SIZE, 1, file);

    if ( result != 1)
    {
      Serial.print("Write result = "); Serial.println(result);
      Serial.print("Write error, i = "); Serial.println(i);

      break;
    }
  }
  
  Serial.println("");
  uint32_t end = millis() - start;
  
  Serial.print(i / 2);
  Serial.print(" Kbytes written in (ms) ");
  Serial.println(end);
  
  fclose(file);

  printLine();

  /////////////////////////////////

  file = fopen(path, "r");
  
  start = millis();
  end = start;
  i = 0;
  
  if (file) 
  {
    start = millis();
    Serial.println("- reading" );

    result = 0;

    fseek(file, 0, SEEK_SET);

    // Read file only 1/4 of NANO33BLE_FS_SIZE_KB
    for (i = 0; i < NANO33BLE_FS_SIZE_KB / 2; i++) 
    {
      result = fread(buf, BUFF_SIZE, 1, file);

      if ( result != 1 )
      {
        Serial.print("Read result = "); Serial.println(result);
        Serial.print("Read error, i = "); Serial.println(i);

        break;
      }
    }
      
    Serial.println("");
    end = millis() - start;
    
    Serial.print((i * BUFF_SIZE) / 1024);
    Serial.print(" Kbytes read in (ms) ");
    Serial.println(end);
    
    fclose(file);
  } 
  else 
  {
    Serial.println("- failed to open file for reading");
  }
}

void printLine()
{
  Serial.println("****************************************************");
}

void setup() 
{
  Serial.begin(115200);
  while (!Serial)

  delay(1000);

  Serial.print("\nStart FS_Test on "); Serial.println(BOARD_NAME);
  Serial.println(FS_NANO33BLE_VERSION);

#if defined(FS_NANO33BLE_VERSION_MIN)
  if (FS_NANO33BLE_VERSION_INT < FS_NANO33BLE_VERSION_MIN)
  {
    Serial.print("Warning. Must use this example on Version equal or later than : ");
    Serial.println(FS_NANO33BLE_VERSION_MIN_TARGET);
  }
#endif

  myFS = new FileSystem_MBED();

  if (!myFS->init())
  {
    Serial.println("FS Mount Failed");
    
    return;
  }

  char fileName1[] = MBED_FS_FILE_PREFIX "/hello1.txt";
  char fileName2[] = MBED_FS_FILE_PREFIX "/hello2.txt";
  
  char message[]  = "Hello from Nano_33_BLE\n";
   
  printLine();
  writeFile(fileName1, message, sizeof(message));
  printLine();
  readFile(fileName1);
  printLine();

  appendFile(fileName1, message, sizeof(message));
  printLine();
  readFile(fileName1);
  printLine();

  renameFile(fileName1, fileName2);
  printLine();
  readCharsFromFile(fileName2);
  printLine();

  deleteFile(fileName2);
  printLine();
  readFile(fileName2);
  printLine();

  testFileIO(fileName1);
  printLine();
  testFileIO(fileName2);
  printLine();
  deleteFile(fileName1);
  printLine();
  deleteFile(fileName2);
  printLine();

  Serial.println( "\nTest complete" );
}

void loop() 
{
}