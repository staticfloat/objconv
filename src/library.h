/****************************  library.h   ********************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2013-08-22
* Project:       objconv
* Module:        library.h
* Description:
* Header file defining classes for reading and writing UNIX and OMF style
* libraries.
*
* Copyright 2007-2013 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#ifndef LIBRARY_H
#define LIBRARY_H


// Make big-endian numbers for library
uint32_t EndianChange(uint32_t);           // Convert little-endian to big-endian number, or vice versa


// Define UNIX library member header
struct SUNIXLibraryHeader {
    char Name[16];                      // Member name
    char Date[12];                      // Member date, seconds, decimal ASCII
    char UserID[6];                     // Member User ID, decimal ASCII
    char GroupID[6];                    // Member Group ID, decimal ASCII
    char FileMode[8];                   // Member file mode, octal
    char FileSize[10];                  // Member file size, decimal ASCII
    char HeaderEnd[2];                  // "`\n"
};


// Class for extracting members from library or building a library
class CLibrary : public CFileBuffer {
public:
    CLibrary();                         // Constructor
    void Go();                          // Do whatever the command line says
    void Dump();                        // Print contents of library
    //static char *TruncateMemberName(char const*);// Remove path and truncate object file name to 15 characters
    static char * ShortenMemberName(char const *name); // Truncate library member name to 15 characters and make unique. The original long name is not overwritten
    static char * StripMemberName(char *);         // Remove path from library member name. Original long name is overwritten
    const char  * GetModuleName(uint32_t Index);     // Get name of module from index or page index
protected:
    // Properties for UNIX input libraries only
    uint32_t LongNames;                   // Offset to long names member
    uint32_t LongNamesSize;               // Size of long names member
    uint32_t AlignBy;                     // Member alignment

    // Properties for OMF input libraries only
    uint32_t PageSize;                    // Alignment of members
    uint32_t DictionaryOffset;            // Offset to hash table
    uint32_t DictionarySize;              // Dictionary size, in 512 bytes blocks

    // Methods and properties for reading library:
    void DumpUNIX();                    // Print contents of UNIX style library
    void DumpOMF();                     // Print contents of OMF style library
    void CheckOMFHash(CMemoryBuffer &stringbuf, CSList<SStringEntry> &index);// Check if OMF library hash table has correct entries for all symbol names
    void StartExtracting();             // Initialize before ExtractMember()
    char * ExtractMember(CFileBuffer*); // Extract next library member from input library
    char * ExtractMemberUNIX(CFileBuffer*); // Extract member of UNIX style library
    char * ExtractMemberOMF(CFileBuffer*);  // Extract member of OMF style library
    uint32_t NextHeader(uint32_t Offset);   // Loop through library headers
    CConverter MemberBuffer;            // Buffer containing single library member
    uint32_t CurrentOffset;               // Offset to current member
    uint32_t CurrentNumber;               // Number of current member
    int  MemberFileType;                // File type of members
    // Methods and properties for modifying or writing library
    void FixNames();                    // Calls StripMemberNamesUNIX or RebuildOMF
    void StripMemberNamesUNIX();        // Remove path from member names
    void RebuildOMF();                  // Rebuild OMF style library to make member names short
    void InsertMember(CFileBuffer*);    // Add next library member to output library
    void InsertMemberUNIX(CFileBuffer*);// Add member to UNIX library
    void InsertMemberOMF(CFileBuffer*); // Add member to OMF library
    void MakeBinaryFile();              // Combine string index and members into binary file
    void MakeBinaryFileUNIX();          // Make UNIX library
    void MakeBinaryFileOMF();           // Make OMF library
    void SortStringTable();             // Sort the string table
    void MakeSymbolTableUnix();         // Make symbol table for COFF, ELF or MACHO library
    CFileBuffer OutFile;                // Buffer for building output file
    CSList<SStringEntry> StringEntries; // String table using SStringEntry
    CMemoryBuffer LongNamesBuffer;      // Buffer for building the "//" longnames member
    CMemoryBuffer StringBuffer;         // Buffer containing strings
    CMemoryBuffer DataBuffer;           // Buffer containing raw members
    CSList<uint32_t> Indexes;             // Buffer containing indexes into DataBuffer
    int RepressWarnings;                // Repress warnings when rebuilding library
};


// Definitions for OMF library hash table:

#define OMFNumBuckets   37             // Number of buckets per block

#define OMFBlockSize    512            // Size of each block

// Structure of hash table block
union SOMFHashBlock {
    struct {
        uint8_t Buckets[OMFNumBuckets];    // Indicators for each bucket
        uint8_t FreeSpace;                 // Pointer to free space
        uint8_t Data[OMFBlockSize-OMFNumBuckets-1]; // Contains strings and module indices
    } b;
    uint8_t Strings[OMFBlockSize];        // Start of each string = length
};


// Hash table handler
class COMFHashTable {
public:
    void Init(SOMFHashBlock * blocks, uint32_t NumBlocks); // Initialize
    void MakeHash(char * name);         // Compute hash
    int  FindString(uint32_t & ModulePage, uint32_t & Conflicts); // Search for string. Get number of occurrences, module, number of conflicting strings
    int  InsertString(uint16_t & ModulePage); // Insert string in hash table. Return 0 if success
    void MakeHashTable(CSList<SStringEntry> & StringEntries, CMemoryBuffer & StringBuffer, CMemoryBuffer & HashTable, CLibrary * Library); // Make hash table
protected:
    char * String;                        // String to search for or insert
    uint32_t StringLength;                // Length of string
    SOMFHashBlock * blocks;               // Pointer to blocks
    uint32_t NumBlocks;                   // Number of blocks
    uint16_t StartBlock;                  // Start block for search
    uint16_t StartBucket;                 // Start bucket for search
    uint16_t BlockD;                      // Block step size in search
    uint16_t BucketD;                     // Bucket step size in search
};

#endif // #ifndef LIBRARY_H
