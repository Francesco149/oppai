#include <Windows.h>
#include <direct.h>
#include <Wincrypt.h>
#define mkdir _mkdir

internalfn
size_t get_exe_path(char* buf, size_t bufsize) {
    return GetModuleFileNameA(0, buf, (u32)bufsize);
}

#define MD5_DIGEST_LENGTH 16

internalfn
void MD5(u8 const* buf, size_t buflen, u8* digest)
{
    HCRYPTPROV prov;

    if (!CryptAcquireContext(&prov, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        fprintf(stderr, "CryptAcquireContext 0x%08X\n", GetLastError());
        return;
    }

    HCRYPTHASH hash;

    if (!CryptCreateHash(prov, CALG_MD5, 0, 0, &hash))
    {
        fprintf(stderr, "CryptCreateHash 0x%08X\n", GetLastError());
        goto cleanup;
    }

    if (!CryptHashData(hash, buf, (DWORD)buflen, 0))
    {
        fprintf(stderr, "CryptHashData 0x%08X\n", GetLastError());
        goto cleanup2;
    }

    DWORD hashlen = MD5_DIGEST_LENGTH;

    if (!CryptGetHashParam(hash, HP_HASHVAL, digest, &hashlen, 0))
    {
        fprintf(stderr, "CryptGetHashParam 0x%08X\n", GetLastError());
    }

cleanup2:
    CryptDestroyHash(hash);

cleanup:
    CryptReleaseContext(prov, 0);
}

