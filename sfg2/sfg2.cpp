#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <locale>

//every size-related variable is in bytes unless specified otherwise

constinit const uint64_t DEFAULT_MEMORY_BLOCK_SIZE = 10ui64 * 1024 * 1024;

void Help();

const uint64_t& GetUnitMultiplier(const char& _unit);

void MakeFile(const char* const _name, const uint64_t& _size, uint64_t _memoryBlockSize = DEFAULT_MEMORY_BLOCK_SIZE);

//exits in case of failure
uint64_t ParseSize(const char* const& _str);

int main(const int _argsCount, const char* _args[])
{
    if (_argsCount < 3 || _argsCount > 4)
        Help();
    MakeFile(_args[1], ParseSize(_args[2]), _argsCount > 3 ? ParseSize(_args[3]) : DEFAULT_MEMORY_BLOCK_SIZE);
    return EXIT_SUCCESS;
}

void Help()
{
    printf
    (
        "Sample File Generator v2\n"
        "- arg1 : filename\n"
        "- arg2 : {file size}{unit}\n"
        "   -> units are b (bytes), k, m, g, t (why)\n"
        "   -> ie 100m\n"
        "- arg3 : {memory block size}{unit}\n"
        "   -> custom memory block size. don't use unless you know what you're doing. default is 10m.\n"
    );
    exit(EXIT_SUCCESS);
}

void MakeFile(const char* const _name, const uint64_t& _size, uint64_t _memoryBlockSize)
{
    if (_size < _memoryBlockSize)
        _memoryBlockSize = _size;
    printf("writing %llu bytes with a memory block of %llu bytes to %s\n", _size, _memoryBlockSize, _name);
    typedef uint8_t byte_t;
    byte_t* _memoryBlock = (byte_t*)malloc(sizeof(byte_t) * _memoryBlockSize);
    if (!_memoryBlock)
    {
        printf("failed to allocate the memory block.");
        exit(EXIT_FAILURE);
    }
    memset(_memoryBlock, 0, _memoryBlockSize);
    FILE* _fp = fopen(_name, "w");
    if (!_fp)
    {
        printf("failed to create/edit the required file.");
        exit(EXIT_FAILURE);
    }
    uint64_t _currentByte{0};
    printf
    (
        "file size : %llu\n"
        "memory block size : %llu\n",
        _size, _memoryBlockSize
    );
    while (_currentByte <= (_size - _memoryBlockSize))
    {
        printf("writing the memory block...\n");
        if (fwrite(_memoryBlock, sizeof(byte_t), _memoryBlockSize, _fp) < _memoryBlockSize)
        {
            printf("failed to write the memory block to the file.");
            fclose(_fp);
            exit(EXIT_FAILURE);
        }
        _currentByte += _memoryBlockSize;
    }
    if (_currentByte < _size)
    {
        printf("writing the filler memory block...\n");
        const uint64_t _fillMemoryBlockSize = _size - _currentByte;
        if (fwrite(_memoryBlock, sizeof(byte_t), _fillMemoryBlockSize, _fp) < _fillMemoryBlockSize)
        {
            printf("failed to write the fill memory block to the file.");
            fclose(_fp);
            exit(EXIT_FAILURE);
        }
    }
    free(_memoryBlock);
    fclose(_fp);
}

const uint64_t& GetUnitMultiplier(const char& _unit)
{
#define INVALID_UNIT 0ui64
#define DECLARE_UNIT(name, value) static constinit const uint64_t ___unit_##name{value};
    DECLARE_UNIT(b, 1ui64)
    DECLARE_UNIT(k, 1024ui64)
    DECLARE_UNIT(m, 1024ui64 * 1024)
    DECLARE_UNIT(g, 1024ui64 * 1024 * 1024)
    DECLARE_UNIT(t, 1024ui64 * 1024 * 1024)
    static constinit const uint64_t _invalid{INVALID_UNIT};
#define DECLARE_UNIT_CASE(name) case #@name : return ___unit_##name;
    switch (_unit)
    {
    DECLARE_UNIT_CASE(b)
    DECLARE_UNIT_CASE(k)
    DECLARE_UNIT_CASE(m)
    DECLARE_UNIT_CASE(g)
    DECLARE_UNIT_CASE(t)
    default: return _invalid;
    }
}

uint64_t ParseSize(const char* const& _str)
{
    const size_t _length = strlen(_str);
    const size_t _lengthPrev = _length - 1;
    const char& _unit = _str[_lengthPrev];
    if
    (
        _length < 2 ||
        GetUnitMultiplier(_unit) == INVALID_UNIT ||
        [&]
        {
            for (size_t i = 0; i < _lengthPrev; ++i)
                if (!std::isdigit(_str[i]))
                    return true;
            return false;
        }
        ()
    )
    {
        printf("invalid size argument.");
        exit(EXIT_FAILURE);
    }
    char* const _nbrStr = (char*)malloc(sizeof(char) * _length);
    _nbrStr[_lengthPrev] = 0;
    strncpy(_nbrStr, _str, _lengthPrev);
    return (uint64_t)strtol(_nbrStr, nullptr, 10) * GetUnitMultiplier(_unit);
}
