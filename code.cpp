#include <iostream>
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h>    // for STDOUT_FILENO
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <iomanip>
#include <bits/stdc++.h>
#include <time.h>
#include <pwd.h>
#include <dirent.h>
using namespace std;
#define cursorforward(x) printf("\033[%dD", (x))
#define cursorbackward(x) printf("\033[%dC", (x))
#define cursorup(x) printf("\033[%dA", (x))
#define cursordown(x) printf("\033[%dB", (x))
#define ESCAPE 0x001b
#define ENTER 0x000a
#define UPARROW 0x0105
#define DOWNARROW 0x0106
#define LEFTARROW 0x0107
#define RIGHTARROW 0x0108
#define KEY_h 0x0068
#define COLON 0x003A
#define home_dir "/home/karan/assignment"
#define BACKSPACE 0x007f
map<int, string> m;
map<string, int> m_rev;
stack<string> L;
stack<string> R;
char esc_start[] = {0x1b, ']', '0', ';', 0};
char esc_end[] = {0x07, 0};
struct items
{
    string name, LM;
    string uid, gid;
    float size;
    string perm;
    char type, size_postfix;
};
bool compare(const items &lhs, const items &rhs)
{
    return lhs.name < rhs.name;
}
string make_rwx(int perm, char type)
{
    int octal[3] = {0};
    int i = 0;
    while (perm != 0)
    {
        octal[i] = perm % 8;
        perm /= 8;
        i++;
    }
    int perm_oct[3] = {0};
    int p = 0;
    for (int j = i - 1; j >= 0; j--)
    {
        perm_oct[p] = octal[j];
        p++;
    }
    string result = "";
    if (type == 'd')
        result = "d";
    else
        result = "-";
    string temp = "";
    for (int k = 0; k < 3; k++)
    {
        switch (perm_oct[k])
        {
        case 0:
            temp = "---";
            break;
        case 1:
            temp = "--x";
            break;
        case 2:
            temp = "-w-";
            break;
        case 3:
            temp = "-wx";
            break;
        case 4:
            temp = "r--";
            break;
        case 5:
            temp = "r-x";
            break;
        case 6:
            temp = "rw-";
            break;
        case 7:
            temp = "rwx";
            break;
        }
        result += temp;
    }
    return result;
}
vector<items> V;
void display()
{
    struct winsize size;
    struct stat s;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    //cout<<"row: "<<size.ws_row<<"\ncol: "<<size.ws_col<<endl;
    //used to clear screen
    system("clear");

    //code for printing content of directory
    static char *pointer = NULL;
    static DIR *dp = NULL;
    int item_count = 0;
    struct dirent *sd = NULL;

    pointer = getenv("PWD");

    dp = opendir((const char *)pointer);
    while ((sd = readdir(dp)) != NULL)
    {

        struct items I;
        stat(sd->d_name, &s);
        string item_name = sd->d_name;
        float item_size = s.st_size;
        string last_modified = ctime(&s.st_mtime);
        unsigned int uid = s.st_uid;
        unsigned int gid = s.st_gid;
        unsigned int item_permission = (s.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
        char type;
        if (S_ISDIR(s.st_mode) == 1)
            type = 'd';
        else
            type = 'f';
        I.type = type;
        I.name = item_name;
        I.LM = last_modified;
        char size_postfix;
        if (int(item_size / 1024) > 0)
        {
            item_size /= 1024;
            if (int(item_size / 1024) > 0)
            {
                item_size /= 1024;
                if (int(item_size / 1024) > 0)
                {
                    item_size /= 1024;
                    size_postfix = 'G';
                }
                else
                    size_postfix = 'M';
            }
            else
                size_postfix = 'K';
            I.size_postfix = size_postfix;
        }
        else
            I.size_postfix = '?';
        I.size = item_size;

        I.uid = getpwuid(uid)->pw_name;
        I.gid = getpwuid(gid)->pw_name;
        I.perm = make_rwx(item_permission, type);
        V.push_back(I);
    }
    sort(V.begin(), V.end(), &compare);
    cout << setbase(10);
    for (auto i : V)
    {
        item_count++;
        m.insert({item_count, i.name});
        m_rev.insert({i.name, item_count});
    }

    for (auto i : V)
    {
        cout << setw(50) << left << i.name << " ";
        if (i.size_postfix != '?')
            cout << right << setw(4) << setprecision(3) << i.size << setw(1) << i.size_postfix << " ";
        else
            cout << right << setw(5) << setprecision(3) << i.size << " ";
        cout << setw(6) << left << i.uid << " "
             << setw(6) << left << i.gid << " "
             << setw(10) << left << i.perm << " "
             << setw(20) << left << i.LM;
    }
}

static int getch(void);
static int kbhit(void);
static int kbesc(void);
static int kbget(void);

static struct termios term, oterm;
static int getch(void)
{
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    return c;
}

static int kbhit(void)
{
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    if (c != -1)
        ungetc(c, stdin);
    return ((c != -1) ? 1 : 0);
}

static int kbesc(void)
{
    int c;

    if (!kbhit())
        return ESCAPE;
    c = getch();
    if (c == '[')
    {
        switch (getch())
        {
        case 'A':
            c = UPARROW;
            break;
        case 'B':
            c = DOWNARROW;
            break;
        case 'C':
            c = RIGHTARROW;
            break;
        case 'D':
            c = LEFTARROW;
            break;
        default:
            c = 0;
            break;
        }
    }
    else
    {
        c = 0;
    }
    if (c == 0)
        while (kbhit())
            getch();
    return c;
}

static int kbget(void)
{
    int c;

    c = getch();
    return (c == ESCAPE) ? kbesc() : c;
}
void delete_dir_recursive(string path)
{

    chdir(path.c_str());
    setenv("PWD", path.c_str(), 1);
    V.clear();
    m.clear();
    display();
    auto v1(V);
    string a = path + "/..";
    chdir(a.c_str());
    setenv("PWD", a.c_str(), 1);
    V.clear();
    m.clear();
    display();
    for (auto i : v1)
    {
        if (i.type == 'd' && i.name != "." && i.name != "..")
        {
            delete_dir_recursive(path + "/" + i.name);
        }
        else
        {
            string word = path + "/" + i.name;
            char *path1 = new char[word.length() + 1];
            strcpy(path1, word.c_str());
            remove(path1);
        }
    }
    rmdir(path.c_str());
}
int search_flag = 0;
string search_recursively(string pattern)
{
    if (search_flag == 1)
        return "True";
    string path = getenv("PWD");
    string orig_path = path;
    V.clear();
    m.clear();
    display();
    auto v1(V);

    for (auto i : v1)
    {
        if (i.type == 'd' && i.name != "." && i.name != "..")
        {
            string word = i.name;
            if (word == pattern)
            {
                search_flag = 1;
                break;
            }
            chdir((path + "/" + i.name).c_str());
            setenv("PWD", (path + "/" + i.name).c_str(), 1);
            if (search_recursively(pattern) == "True")
            {
                search_flag = 1;
                break;
            }
        }
        else
        {
            string word = i.name;
            if (word == pattern)
            {
                search_flag = 1;
                break;
            }
        }
    }
    string a = orig_path;
    chdir(a.c_str());
    setenv("PWD", a.c_str(), 1);
    V.clear();
    m.clear();
    display();
    if (search_flag == 1)
        return "True";
    else
        return "False";
}
int main()
{
    cout << esc_start << "NORMAL MODE" << esc_end;
    int c, flag = 0;
    while (1)
    {
        int count;
        if (flag == 0)
        {
            flag = 1;
            system("clear");
            V.clear();
            m.clear();
            count = 0;
            display();
            count = m.size() + 1;
        }
        c = kbget();
        if (c == ENTER)
        {

            if (V[count - 1].type == 'd')
            {
                L.push(getenv("PWD"));
                string a = getenv("PWD");
                string b = "/" + V[count - 1].name;
                a += b;
                int i = 0;
                char newpath[a.length() + 1];
                for (i = 0; i < a.length(); i++)
                    newpath[i] = a[i];
                newpath[i] = '\0';
                chdir(newpath);
                setenv("PWD", newpath, 1);

                system("clear");
                flag = 0;
            }
            else
            {
                L.push(getenv("PWD"));
                string a = "vi ";
                a += getenv("PWD");

                a += "/" + V[count - 1].name;

                int i = 0;
                char newpath[a.length() + 1];
                for (i = 0; i < a.length(); i++)
                    newpath[i] = a[i];
                newpath[i] = '\0';
                system(newpath);
            }
        }
        else if (c == KEY_h)
        {
            L.push(getenv("PWD"));
            string a = home_dir;
            int i = 0;
            char newpath[a.length() + 1];
            for (i = 0; i < a.length(); i++)
                newpath[i] = a[i];
            newpath[i] = '\0';
            chdir(newpath);
            setenv("PWD", newpath, 1);
            system("clear");
            flag = 0;
        }
        else if (c == BACKSPACE)
        {
            L.push(getenv("PWD"));
            string a = getenv("PWD");
            string b = "/..";
            a += b;
            int i = 0;
            char newpath[a.length() + 1];
            for (i = 0; i < a.length(); i++)
                newpath[i] = a[i];
            newpath[i] = '\0';
            chdir(newpath);
            setenv("PWD", newpath, 1);
            system("clear");
            flag = 0;
        }
        else if (c == UPARROW)
        {
            cursorup(1);
            count--;
        }
        else if (c == DOWNARROW)
        {
            cursordown(1);
            count++;
        }
        else if (c == RIGHTARROW)
        {
            if (R.empty())
                continue;
            string a = R.top();
            R.pop();
            L.push(getenv("PWD"));
            int i = 0;
            char newpath[a.length() + 1];
            for (i = 0; i < a.length(); i++)
                newpath[i] = a[i];
            newpath[i] = '\0';
            chdir(newpath);
            setenv("PWD", newpath, 1);
            flag = 0;
        }
        else if (c == LEFTARROW)
        {
            if (L.empty())
                continue;
            string a = L.top();
            L.pop();
            R.push(getenv("PWD"));
            int i = 0;
            char newpath[a.length() + 1];
            for (i = 0; i < a.length(); i++)
                newpath[i] = a[i];
            newpath[i] = '\0';
            chdir(newpath);
            setenv("PWD", newpath, 1);
            flag = 0;
        }
        else if (c == COLON)
        {
            struct winsize size;
            struct stat s;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
            int lastline = size.ws_row + 1;
            int esc_flag = 0;
            while (1)
            {
                if (esc_flag == 1)
                    break;

                cout << esc_start << "COMMAND MODE" << esc_end;
                printf("%c[%d;%dH", 27, lastline, 1);
                printf("%c[2K", 27);
                cout << ":";
                int flag = 0;
                string input;
                while (1)
                {
                    int c;
                    flag = 0;

                    tcgetattr(0, &oterm);
                    memcpy(&term, &oterm, sizeof(term));
                    term.c_lflag &= ~(ICANON | ECHO);
                    term.c_cc[VMIN] = 1;
                    term.c_cc[VTIME] = 0;
                    tcsetattr(0, TCSANOW, &term);
                    c = getchar();
                    tcsetattr(0, TCSANOW, &oterm);

                    if (c == 27)
                    {
                        esc_flag = 1;
                        break;
                    }
                    else if (c == 10)
                    {
                        flag = 1;
                        break;
                    }
                    else if (c == 127)
                    {

                        printf("%c[%d;%dH", 27, lastline, 1);
                        printf("%c[2K", 27);
                        cout << ":";
                        if (input.length() <= 1)
                        {
                            input = "";
                        }
                        else
                        {
                            input = input.substr(0, input.length() - 1);
                        }
                        cout << input;
                    }
                    else
                    {
                        input = input + char(c);
                        cout << char(c);
                    }
                }
                if (flag == 1)
                {
                    vector<string> v;
                    string temp = "";
                    for (auto i : input)
                    {
                        if (i == ' ')
                        {
                            v.push_back(temp);
                            temp = "";
                        }
                        else
                        {
                            temp += i;
                        }
                    }
                    v.push_back(temp);
                    if (v[0] == "copy")
                    {

                        vector<string> source_vector;
                        for (auto it = v.begin() + 1; it != v.end() - 1; it++)
                        {
                            source_vector.push_back(*it);
                        }
                        for (string source : source_vector)
                        {
                            string destination = home_dir + (*(v.end() - 1)).substr(1, input.length() - 1) + "/" + source;
                            char block[1024];
                            int in, out;
                            int nread;
                            char *path = new char[source.length() + 1];
                            strcpy(path, source.c_str());
                            char *des = new char[destination.length() + 1];
                            strcpy(des, destination.c_str());

                            in = open(path, O_RDONLY);
                            out = open(des, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                            while ((nread = read(in, block, sizeof(block))) > 0)
                                write(out, block, nread);
                        }
                    }
                    else if (v[0] == "move")
                    {
                        vector<string> source_vector;

                        for (auto it = v.begin() + 1; it != v.end() - 1; it++)
                        {
                            source_vector.push_back(*it);
                        }
                        for (string source : source_vector)
                        {
                            string destination = home_dir + (*(v.end() - 1)).substr(1, input.length() - 1) + "/" + source;
                            char block[1024];
                            int in, out;
                            int nread;
                            char *path = new char[source.length() + 1];
                            strcpy(path, source.c_str());
                            char *des = new char[destination.length() + 1];
                            strcpy(des, destination.c_str());

                            in = open(path, O_RDONLY);
                            out = open(des, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                            while ((nread = read(in, block, sizeof(block))) > 0)
                                write(out, block, nread);
                            char *path1 = new char[source.length() + 1];
                            strcpy(path1, source.c_str());
                            remove(path1);
                        }
                    }
                    else if (v[0] == "rename")
                    {
                        string source = v[1];
                        string destination = v[2];
                        char block[1024];
                        int in, out;
                        int nread;
                        char *path = new char[source.length() + 1];
                        strcpy(path, source.c_str());
                        char *des = new char[destination.length() + 1];
                        strcpy(des, destination.c_str());
                        in = open(path, O_RDONLY);
                        out = open(des, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                        while ((nread = read(in, block, sizeof(block))) > 0)
                            write(out, block, nread);
                        char *path1 = new char[source.length() + 1];
                        strcpy(path1, source.c_str());
                        remove(path1);
                    }
                    else if (v[0] == "create_file")
                    {
                        string source = v[1];
                        string destination = home_dir + (*(v.end() - 1)).substr(1, input.length() - 1) + "/" + source;
                        char *des = new char[destination.length() + 1];
                        strcpy(des, destination.c_str());
                        open(des, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    }
                    else if (v[0] == "create_dir")
                    {
                        string source = v[1];
                        string destination = home_dir + (v[2]).substr(1, input.length() - 1) + "/" + source;
                        mkdir(destination.c_str(), 0777);
                    }
                    else if (v[0] == "delete_file")
                    {
                        string source = v[1];
                        char *path = new char[source.length() + 1];
                        strcpy(path, source.c_str());
                        remove(path);
                    }
                    else if (v[0] == "delete_dir")
                    {
                        string source = v[1];
                        string a = getenv("PWD");
                        string original_dir = a;
                        string b = "/" + source;
                        a += b;
                        delete_dir_recursive(a);
                        V.clear();
                        m.clear();
                        system("clear");
                        chdir(original_dir.c_str());
                        setenv("PWD", original_dir.c_str(), 1);
                        display();
                    }
                    else if (v[0] == "goto")
                    {
                        string source = home_dir + v[1].substr(1, input.length() - 1);
                        L.push(getenv("PWD"));
                        system("clear");
                        char *path = new char[source.length() + 1];
                        strcpy(path, source.c_str());
                        chdir(path);
                        setenv("PWD", path, 1);
                        V.clear();
                        m.clear();
                        display();
                        count = m.size() + 1;
                    }
                    else if (v[0] == "search")
                    {
                        string source = v[1];
                        cout << search_recursively(source) << endl;
                        printf("%c[%d;%dH", 27, lastline - 1, 0);
                        printf("%c[2K", 27);
                        search_flag = 0;
                    }
                }

                printf("%c[%d;%dH", 27, lastline, 0);
                printf("%c[2K", 27);
                cout << ":";
            }
            int lastline1 = count;
            printf("%c[%d;%dH", 27, lastline1, 0);
            printf("%c[2K", 27);
            cout << esc_start << "NORMAL MODE" << esc_end;
        }
        else
        {
            putchar(c);
        }
    }
    printf("\n");
    return 0;
}
