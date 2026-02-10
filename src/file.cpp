/**
 * @Authors         Tom Keuper
 * @Date created    23-10-2024
 * @Date updated    25-10-2024 (By: Tom Keuper)
 * @Description     File system controller for reading and writing to the LittleFS file system
 */

#include "C1001Controller.h"

#define FS_BUFSIZE 256

// There are no consecutive spaces longer than this in the file, so if more space is required, findSpace() can return false immediately
// Actual space may be lower
constexpr size_t MAX_SPACE = UINT16_MAX * 2U;           // smallest supported config has 128Kb flash size
static volatile size_t knownLargestSpace = MAX_SPACE;

static File f; // don't export to other cpp files

//wrapper to find out how long closing takes
void closeFile() {
#ifdef ESP_DEBUG_FS
    DEBUGFS_PRINT(F("Close -> "));
    uint32_t s = millis();
#endif
    f.close();
    DEBUGFS_PRINTF("took %d ms\n", millis() - s);
    doCloseFile = false;
}


//find() that reads and buffers data from file stream in 256-byte blocks.
//Significantly faster, f.find(key) can take SECONDS for multi-kB files
static bool bufferedFind(const char *target, bool fromStart = true) {
  #ifdef ESP_DEBUG_FS
    DEBUGFS_PRINT("Find ");
    DEBUGFS_PRINTLN(target);
    uint32_t s = millis();
  #endif

  if (!f || !f.size()) return false;
  size_t targetLen = strlen(target);

  size_t index = 0;
  byte buf[FS_BUFSIZE];
  if (fromStart) f.seek(0);

  while (f.position() < f.size() -1) {
    size_t bufsize = f.read(buf, FS_BUFSIZE); // better to use size_t instead if uint16_t
    size_t count = 0;
    while (count < bufsize) {
      if(buf[count] != target[index])
      index = 0; // reset index if any char does not match

      if(buf[count] == target[index]) {
        if(++index >= targetLen) { // return true if all chars in the target match
          f.seek((f.position() - bufsize) + count +1);
          DEBUGFS_PRINTF("Found at pos %d, took %d ms", f.position(), millis() - s);
          return true;
        }
      }
      count++;
    }
  }
  DEBUGFS_PRINTF("No match, took %d ms\n", millis() - s);
  return false;
}

//find empty spots in file stream in 256-byte blocks.
static bool bufferedFindSpace(size_t targetLen, bool fromStart = true) {

  #ifdef ESP_DEBUG_FS
    DEBUGFS_PRINTF("Find %d spaces\n", targetLen);
    uint32_t s = millis();
  #endif

  if (knownLargestSpace < targetLen) {
    DEBUGFS_PRINT(F("No match, KLS "));
    DEBUGFS_PRINTLN(knownLargestSpace);
    return false;
  }

  if (!f || !f.size()) return false;

  size_t index = 0; // better to use size_t instead if uint16_t
  byte buf[FS_BUFSIZE];
  if (fromStart) f.seek(0);

  while (f.position() < f.size() -1) {
    size_t bufsize = f.read(buf, FS_BUFSIZE);
    size_t count = 0;

    while (count < bufsize) {
      if(buf[count] == ' ') {
        if(++index >= targetLen) { // return true if space long enough
          if (fromStart) {
            f.seek((f.position() - bufsize) + count +1 - targetLen);
            knownLargestSpace = MAX_SPACE; //there may be larger spaces after, so we don't know
          }
          DEBUGFS_PRINTF("Found at pos %d, took %d ms", f.position(), millis() - s);
          return true;
        }
      } else {
        if (!fromStart) return false;
        if (index) {
          if (knownLargestSpace < index || (knownLargestSpace == MAX_SPACE)) knownLargestSpace = index;
          index = 0; // reset index if not space
        }
      }

      count++;
    }
  }
  DEBUGFS_PRINTF("No match, took %d ms\n", millis() - s);
  return false;
}

//find the closing bracket corresponding to the opening bracket at the file pos when calling this function
static bool bufferedFindObjectEnd() {
  #ifdef ESP_DEBUG_FS
    DEBUGFS_PRINTLN(F("Find obj end"));
    uint32_t s = millis();
  #endif

  if (!f || !f.size()) return false;

  uint16_t objDepth = 0; //num of '{' minus num of '}'. return once 0
  //size_t start = f.position();
  byte buf[FS_BUFSIZE];

  while (f.position() < f.size() -1) {
    size_t bufsize = f.read(buf, FS_BUFSIZE); // better to use size_t instead of uint16_t
    size_t count = 0;

    while (count < bufsize) {
      if (buf[count] == '{') objDepth++;
      if (buf[count] == '}') objDepth--;
      if (objDepth == 0) {
        f.seek((f.position() - bufsize) + count +1);
        DEBUGFS_PRINTF("} at pos %d, took %d ms", f.position(), millis() - s);
        return true;
      }
      count++;
    }
  }
  DEBUGFS_PRINTF("No match, took %d ms\n", millis() - s);
  return false;
}

//fills n bytes from current file pos with ' ' characters
static void writeSpace(size_t l)
{
  byte buf[FS_BUFSIZE];
  memset(buf, ' ', FS_BUFSIZE);

  while (l > 0) {
    size_t block = (l>FS_BUFSIZE) ? FS_BUFSIZE : l;
    f.write(buf, block);
    l -= block;
  }

  if (knownLargestSpace < l) knownLargestSpace = l;
}

bool appendObjectToFile(const char* key, JsonDocument* content, uint32_t s, uint32_t contentLen = 0)
{
  #ifdef ESP_DEBUG_FS
    DEBUGFS_PRINTLN(F("Append"));
    uint32_t s1 = millis();
  #endif
  uint32_t pos = 0;
  if (!f) return false;

  if (f.size() < 3) {
    char init[10];
    strcpy_P(init, PSTR("{\"0\":{}}"));
    f.print(init);
  }

  if (content->isNull()) {
    doCloseFile = true;
    return true; //nothing  to append
  }

  //if there is enough empty space in file, insert there instead of appending
  if (!contentLen) contentLen = measureJson(*content);
  DEBUGFS_PRINTF("CLen %d\n", contentLen);
  if (bufferedFindSpace(contentLen + strlen(key) + 1)) {
    if (f.position() > 2) f.write(','); //add comma if not first object
    f.print(key);
    serializeJson(*content, f);
    DEBUGFS_PRINTF("Inserted, took %d ms (total %d)", millis() - s1, millis() - s);
    doCloseFile = true;
    return true;
  }

  //not enough space, append at end

  //permitted space for presets exceeded
  updateFSInfo();

  if (f.size() + 9000 > (fsBytesTotal - fsBytesUsed)) { //make sure there is enough space to at least copy the file once
    DEBUG_PRINT("-=-=- QUOTA EXCEEDED! -=-=-");
    doCloseFile = true;
    return false;
  }

  //check if last character in file is '}' (typical)
  uint32_t eof = f.size() -1;
  f.seek(eof, SeekSet);
  if (f.read() == '}') pos = eof;

  if (pos == 0) //not found
  {
    DEBUGFS_PRINTLN("not }");
    f.seek(0);
    while (bufferedFind("}",false)) //find last closing bracket in JSON if not last char
    {
      pos = f.position();
    }
    if (pos > 0) pos--;
  }
  DEBUGFS_PRINT("pos "); DEBUGFS_PRINTLN(pos);
  if (pos > 2)
  {
    f.seek(pos, SeekSet);
    f.write(',');
  } else { //file content is not valid JSON object
    f.seek(0, SeekSet);
    f.print('{'); //start JSON
  }

  f.print(key);

  //Append object
  serializeJson(*content, f);
  f.write('}');

  doCloseFile = true;
  DEBUGFS_PRINTF("Appended, took %d ms (total %d)", millis() - s1, millis() - s);
  return true;
}

bool writeObjectToFileUsingId(const char* file, uint16_t id, JsonDocument* content)
{
  char objKey[10];
  sprintf(objKey, "\"%d\":", id);
  return writeObjectToFile(file, objKey, content);
}

bool writeObjectToFile(const char* file, const char* key, JsonDocument* content)
{
  uint32_t s = 0; //timing
  #ifdef ESP_DEBUG_FS
    DEBUGFS_PRINTF("Write to %s with key %s >>>\n", file, (key==nullptr)?"nullptr":key);
    serializeJson(*content, Serial); DEBUGFS_PRINTLN();
    s = millis();
  #endif

  size_t pos = 0;
  f = LittleFS.open(file, "r+");
  if (!f && !LittleFS.exists(file)) f = LittleFS.open(file, "w+");
  if (!f) {
    DEBUGFS_PRINTLN(F("Failed to open!"));
    return false;
  }

  if (!bufferedFind(key)) //key does not exist in file
  {
    return appendObjectToFile(key, content, s);
  }

  //an object with this key already exists, replace or delete it
  pos = f.position();
  //measure out end of old object
  bufferedFindObjectEnd();
  size_t pos2 = f.position();

  uint32_t oldLen = pos2 - pos;
  DEBUGFS_PRINTF("Old obj len %d\n", oldLen);

  //Three cases:
  //1. The new content is null, overwrite old obj with spaces
  //2. The new content is smaller than the old, overwrite and fill diff with spaces
  //3. The new content is larger than the old, but smaller than old + trailing spaces, overwrite with new
  //4. The new content is larger than old + trailing spaces, delete old and append

  size_t contentLen = 0;
  if (!content->isNull()) contentLen = measureJson(*content);

  if (contentLen && contentLen <= oldLen) { //replace and fill diff with spaces
    DEBUGFS_PRINTLN(F("replace"));
    f.seek(pos);
    serializeJson(*content, f);
    writeSpace(pos2 - f.position());
  } else if (contentLen && bufferedFindSpace(contentLen - oldLen, false)) { //enough leading spaces to replace
    DEBUGFS_PRINTLN(F("replace (trailing)"));
    f.seek(pos);
    serializeJson(*content, f);
  } else {
    DEBUGFS_PRINTLN(F("delete"));
    pos -= strlen(key);
    if (pos > 3) pos--; //also delete leading comma if not first object
    f.seek(pos);
    writeSpace(pos2 - pos);
    if (contentLen) return appendObjectToFile(key, content, s, contentLen);
  }

  doCloseFile = true;
  DEBUGFS_PRINTF("Replaced/deleted, took %d ms\n", millis() - s);
  return true;
}

bool readObjectFromFileUsingId(const char* file, uint16_t id, JsonDocument* dest)
{
  char objKey[10];
  sprintf(objKey, "\"%d\":", id);
  return readObjectFromFile(file, objKey, dest);
}

//if the key is a nullptr, deserialize entire object
bool readObjectFromFile(const char* file, const char* key, JsonDocument* dest)
{
  if (doCloseFile) closeFile();
  #ifdef ESP_DEBUG_FS
    DEBUGFS_PRINTF("Read from %s with key %s >>>\n", file, (key==nullptr)?"nullptr":key);
    uint32_t s = millis();
  #endif

  f = LittleFS.open(file, "r");
  if (!f) {
    DEBUGFS_PRINTLN(F("Failed to open file."));
    return false;
  }

#ifdef ESP_DEBUG_FS
  // Debugging: Print file contents
  String fileContents = f.readString();
  DEBUGFS_PRINTF("File contents:\n%s\n", fileContents.c_str());
  // Reset file position for deserialization
  f.seek(0, SeekSet);
#endif

  if (key != nullptr && !bufferedFind(key)) // Key does not exist in file
  {
    f.close();
    dest->clear();
    DEBUGFS_PRINTLN(F("Obj not found."));
    return false;
  }

  // Deserialize JSON from file
  DeserializationError error = deserializeJson(*dest, f);
  if (error) {
    DEBUGFS_PRINTF("Deserialization failed: %s\n", error.c_str());
    f.close();
    return false;
  }

  DEBUGFS_PRINTF("Number of bytes read: %d\n", f.position());
  f.close();

  #ifdef ESP_DEBUG_FS
    DEBUGFS_PRINTF("Read, took %d ms\n", millis() - s);
  #endif

  return true;
}

void updateFSInfo() {
  fsBytesTotal = LittleFS.totalBytes();
  fsBytesUsed = LittleFS.usedBytes();
}


//Un-comment any file types you need
static String getContentType(AsyncWebServerRequest* request, String filename){
  if(filename.endsWith(".json")) return "application/json";
//  else if(filename.endsWith(".htm")) return "text/html";
//  else if(filename.endsWith(".html")) return "text/html";
//  else if(filename.endsWith(".css")) return "text/css";
//  else if(filename.endsWith(".js")) return "application/javascript";
  else if(request->hasArg("download")) return "application/octet-stream";
//  else if(filename.endsWith(".png")) return "image/png";
//  else if(filename.endsWith(".gif")) return "image/gif";
//  else if(filename.endsWith(".jpg")) return "image/jpeg";
//  else if(filename.endsWith(".ico")) return "image/x-icon";
//  else if(filename.endsWith(".xml")) return "text/xml";
//  else if(filename.endsWith(".pdf")) return "application/x-pdf";
//  else if(filename.endsWith(".zip")) return "application/x-zip";
//  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(AsyncWebServerRequest* request, String path){
  DEBUG_PRINTLN("WS FileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  if(path.indexOf("sec") > -1) return false;
  String contentType = getContentType(request, path);
  /*String pathWithGz = path + ".gz";
  if(LittleFS.exists(pathWithGz)){
    request->send(LittleFS, pathWithGz, contentType);
    return true;
  }*/
  if(LittleFS.exists(path)) {
    request->send(LittleFS, path, contentType);
    return true;
  }
  return false;
}