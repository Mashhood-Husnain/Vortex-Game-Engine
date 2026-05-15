#include "util/vortex_save_load.hpp"
#include "util/vortex_logs.hpp"

std::string VortexEncrypt::get_key()
{
    std::ifstream env_file(".env");
    std::string line;

    if (env_file.is_open())
    {
        while(std::getline(env_file, line))
        {
            if (line.find("ENCRYPTION_KEY") == 0) return line.substr(15);
        }

        env_file.close();
    }

    VORTEX_WARN("[ENCRYPTION] .env file or ENCRYPTION_KEY not found. Using default key");
    return "VORTEX_DEFAULT_SECURE_KEY_9921";
}

std::string _xor_(std::string data, std::string key)
{
    for (size_t i = 0; i < data.size(); i++)
    {
        data[i] ^= key[i % key.length()];
    }

    return data;
}

void VortexEncrypt::write_encrypted(const std::string &file_path, const std::string &data)
{
    std::string key = get_key();
    std::string encrypted_data = _xor_(data, key);

    std::ofstream out_file(file_path, std::ios::binary | std::ios::trunc);
    if (out_file)
    {
        out_file.write(encrypted_data.c_str(), encrypted_data.size());
        out_file.close();
    }
}

std::string VortexEncrypt::read_decrypted(const std::string &file_path)
{
    std::string key = get_key();

    std::ifstream in_file(file_path, std::ios::binary);
    if(!in_file) return "";

    std::string data((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());
    in_file.close();

    data = _xor_(data, key);

    return data;
}
