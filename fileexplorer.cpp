#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <dirent.h>
#include <iomanip>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <fstream>
#include <cstdlib>

using namespace std;

/**
 * @brief Converts file size in bytes to human-readable format.
 */
string formatSize(long long size_bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = static_cast<double>(size_bytes);
    while (size >= 1024 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }

    stringstream ss;
    if (unit_index > 0)
        ss << fixed << setprecision(2) << size << " " << units[unit_index];
    else
        ss << size_bytes << " " << units[unit_index];
    return ss.str();
}

/**
 * @brief Get the string representation of the file type.
 */
string getFileType(mode_t mode) {
    if (S_ISDIR(mode)) return "DIR";
    if (S_ISREG(mode)) return "FILE";
    if (S_ISLNK(mode)) return "LINK";
    if (S_ISCHR(mode)) return "CHAR";
    if (S_ISBLK(mode)) return "BLCK";
    if (S_ISFIFO(mode)) return "FIFO";
    if (S_ISSOCK(mode)) return "SOCK";
    return "OTHR";
}

/**
 * @brief Lists the contents of the given directory path.
 */
bool listDirectory(const string& path) {
    DIR *dirp = opendir(path.c_str());
    if (dirp == NULL) {
        cerr << "❌ Error: Could not open directory " << path << "\n";
        return false;
    }

    struct dirent *dp;
    struct stat filestat;

    cout << left << setw(8) << "TYPE" << setw(12) << "SIZE" << "NAME\n";
    cout << string(40, '-') << "\n";

    while ((dp = readdir(dirp)) != NULL) {
        string name = dp->d_name;
        string fullPath = path + "/" + name;
        if (stat(fullPath.c_str(), &filestat) == 0) {
            cout << left << setw(8) << getFileType(filestat.st_mode)
                 << setw(12) << formatSize(filestat.st_size)
                 << name << "\n";
        }
    }
    closedir(dirp);
    return true;
}

/**
 * @brief Copies a file.
 */
bool copyFile(const string& src, const string& dest) {
    ifstream in(src, ios::binary);
    ofstream out(dest, ios::binary);
    if (!in || !out) return false;
    out << in.rdbuf();
    return true;
}

/**
 * @brief Searches recursively for a file or directory name.
 */
void searchItem(const string& basePath, const string& name) {
    DIR* dirp = opendir(basePath.c_str());
    if (!dirp) return;

    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
        string path = basePath + "/" + dp->d_name;
        if (name == dp->d_name)
            cout << "Found: " << path << "\n";

        struct stat st;
        if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
            searchItem(path, name);
    }
    closedir(dirp);
}

/**
 * @brief Displays permissions of a file.
 */
void showPermissions(const string& name) {
    struct stat st;
    if (stat(name.c_str(), &st) != 0) {
        cout << "❌ Cannot access: " << name << endl;
        return;
    }
    mode_t p = st.st_mode;
    cout << ((p & S_IRUSR) ? "r" : "-");
    cout << ((p & S_IWUSR) ? "w" : "-");
    cout << ((p & S_IXUSR) ? "x" : "-");
    cout << ((p & S_IRGRP) ? "r" : "-");
    cout << ((p & S_IWGRP) ? "w" : "-");
    cout << ((p & S_IXGRP) ? "x" : "-");
    cout << ((p & S_IROTH) ? "r" : "-");
    cout << ((p & S_IWOTH) ? "w" : "-");
    cout << ((p & S_IXOTH) ? "x" : "-") << "\n";
}

/**
 * @brief Displays available commands.
 */
void showHelp() {
    cout << "\n📘 Commands Available:\n";
    cout << "ls                    - List files in current directory\n";
    cout << "cd <dir>              - Change directory\n";
    cout << "mkdir <name>          - Create a new directory\n";
    cout << "touch <file>          - Create a new empty file\n";
    cout << "rm <file>             - Delete a file\n";
    cout << "rmdir <dir>           - Delete a directory\n";
    cout << "cp <src> <dest>       - Copy file\n";
    cout << "mv <src> <dest>       - Move/Rename file or folder\n";
    cout << "search <name>         - Search for a file/folder\n";
    cout << "perm <name>           - Show permissions\n";
    cout << "chmod <mode> <file>   - Change permissions (e.g. 755)\n";
    cout << "help                  - Show help menu\n";
    cout << "exit                  - Exit File Explorer\n";
}

/**
 * @brief Main interactive File Explorer loop.
 */
int main() {
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    string currentPath = cwd;

    cout << "\ninput any command \n";
    cout << "Type 'help' to see available commands.\n";

    string input;
    while (true) {
        cout << "\n[" << currentPath << "]$ ";
        getline(cin, input);
        if (input.empty()) continue;

        stringstream ss(input);
        string cmd, arg1, arg2;
        ss >> cmd >> arg1 >> arg2;

        if (cmd == "exit") break;
        else if (cmd == "ls") listDirectory(currentPath);
        else if (cmd == "cd") {
            string newPath = (arg1 == "..") ? (currentPath.substr(0, currentPath.find_last_of('/'))) : (currentPath + "/" + arg1);
            if (chdir(newPath.c_str()) == 0) {
                char temp[PATH_MAX];
                getcwd(temp, sizeof(temp));
                currentPath = temp;
            } else cout << "❌ Directory not found.\n";
        }
        else if (cmd == "mkdir") {
            if (mkdir((currentPath + "/" + arg1).c_str(), 0755) == 0)
                cout << "✅ Directory created.\n";
            else
                cout << "❌ Failed to create directory.\n";
        }
        else if (cmd == "touch") {
            ofstream file(currentPath + "/" + arg1);
            if (file) cout << "✅ File created.\n";
            else cout << "❌ Could not create file.\n";
        }
        else if (cmd == "rm") {
            if (remove((currentPath + "/" + arg1).c_str()) == 0)
                cout << "🗑️ File deleted.\n";
            else
                cout << "❌ Could not delete file.\n";
        }
        else if (cmd == "rmdir") {
            if (rmdir((currentPath + "/" + arg1).c_str()) == 0)
                cout << "🗑️ Directory removed.\n";
            else
                cout << "❌ Could not remove directory.\n";
        }
        else if (cmd == "cp") {
            string src = currentPath + "/" + arg1;
            string dest = currentPath + "/" + arg2;
            if (copyFile(src, dest))
                cout << "📋 Copy successful.\n";
            else
                cout << "❌ Copy failed.\n";
        }
        else if (cmd == "mv") {
            string src = currentPath + "/" + arg1;
            string dest = currentPath + "/" + arg2;
            if (rename(src.c_str(), dest.c_str()) == 0)
                cout << "📦 Move successful.\n";
            else
                cout << "❌ Move failed.\n";
        }
        else if (cmd == "search") searchItem(currentPath, arg1);
        else if (cmd == "perm") showPermissions(currentPath + "/" + arg1);
        else if (cmd == "chmod") {
            if (chmod((currentPath + "/" + arg2).c_str(), stoi(arg1, 0, 8)) == 0)
                cout << "✅ Permissions updated.\n";
            else
                cout << "❌ Failed to update permissions.\n";
        }
        else if (cmd == "help") showHelp();
        else cout << "❓ Unknown command. Type 'help' for options.\n";
    }

    cout << "\n👋 Exiting File Explorer.\n";
    return 0;
}
