typedef unsigned long fw_addr_t;

extern void CoroutineEntry(void);
extern void near NearCoroutineEntry(void);

fw_addr_t ResolveCoroutineEntryAddress(void)
{
    return (fw_addr_t)CoroutineEntry;
}

fw_addr_t ResolveNearCoroutineEntryAddress(void)
{
    return (fw_addr_t)NearCoroutineEntry;
}
