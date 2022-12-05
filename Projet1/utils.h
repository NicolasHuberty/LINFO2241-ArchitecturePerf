struct client_pkt{
    uint32_t index;
    uint32_t key_size;
    int* key;
};
struct server_pkt{
    uint8_t error;
    uint32_t file_size;
    char* encrypted_file;
};