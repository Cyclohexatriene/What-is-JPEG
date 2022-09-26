#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <sstream>
#include <queue>
#include <unordered_map>
#include <unordered_set>
using namespace std;
#define SIZE 256 //Size of image
#define PI 3.14159265358979323846   // pi

int load(string path, vector<vector<vector<double>>>& pixels) {
    ifstream ifs("demo.bin", ios::in | ios::binary); //Open binary file.
    if (!ifs.is_open()) return -1; //Check whether the file is successfully opened. If not, the program will exit with code -1.
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int bufR = 0, bufG = 0, bufB = 0;
            ifs.read((char*)&bufB, sizeof(char)); //Read 1 Byte at a time.
            ifs.read((char*)&bufG, sizeof(char));
            ifs.read((char*)&bufR, sizeof(char));
            pixels[i][j][0] = (double)bufR;
            pixels[i][j][1] = (double)bufG;
            pixels[i][j][2] = (double)bufB;
        }
    }
    ifs.close();
    return 0;
}

void colorSpaceConversion(vector<vector<vector<double>>>& pixels) {
    //RGB to YCrCb. 
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            double r = pixels[i][j][0];
            double g = pixels[i][j][1];
            double b = pixels[i][j][2];

            //Subtract 128 in advance.
            double Y = 0.299 * r + 0.587 * g + 0.114 * b - 128;
            double Cr = 0.5 * r - 0.4187 * g - 0.0813 * b;
            double Cb = -0.1687 * r - 0.3313 * g + 0.5 * b;

            pixels[i][j][0] = Y;
            pixels[i][j][1] = Cr;
            pixels[i][j][2] = Cb;
        }
    }
    return;
}

inline double C(int x) { return x == 0 ? 1 / sqrt(2) : 1; }
void DCT(vector<vector<vector<double>>>& pixels) {
    for (int u = 0; u < SIZE; u += 8) {
        for (int v = 0; v < SIZE; v += 8) {
            for (int ut = 0; ut < 8; ut++) {
                for (int vt = 0; vt < 8; vt++) {
                    double tempY = 0, tempCr = 0, tempCb = 0;
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            double co = cos((2 * i + 1) * ut * PI / 16) * cos((2 * j + 1) * vt * PI / 16);
                            tempY += pixels[u + i][v + j][0] * co;
                            tempCr += pixels[u + i][v + j][1] * co;
                            tempCb += pixels[u + i][v + j][2] * co;
                        }
                    }
                    pixels[u + ut][v + vt][0] = 0.25 * C(ut) * C(vt) * tempY;
                    pixels[u + ut][v + vt][1] = 0.25 * C(ut) * C(vt) * tempCr;
                    pixels[u + ut][v + vt][2] = 0.25 * C(ut) * C(vt) * tempCb;
                }
            }
            
        }
    }
    return;
}

void quantify(vector<vector<vector<double>>>& pixels) {
    vector<vector<int>> QY = {
        {16,11,10,16,24,40,51,61},
        {12,12,14,19,26,58,60,55},
        {14,13,16,24,40,57,69,56},
        {14,17,22,29,51,87,80,62},
        {18,22,37,56,68,109,103,77},
        {24,35,55,64,81,104,113,92},
        {49,64,78,87,103,121,120,101},
        {72,92,95,98,112,100,103,99}
    };
    vector<vector<int>> QC = {
        {17,18,24,47,99,99,99,99},
        {18,21,26,66,88,99,99,99},
        {24,26,56,99,99,99,99,99},
        {47,66,99,99,99,99,99,99},
        {99,99,99,99,99,99,99,99},
        {99,99,99,99,99,99,99,99},
        {99,99,99,99,99,99,99,99},
        {99,99,99,99,99,99,99,99},
    };
    for (int u = 0; u < SIZE; u += 8) {
        for (int v = 0; v < SIZE; v += 8) {
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    pixels[u + i][v + j][0] = (double)(int)(pixels[u + i][v + j][0] / QY[i][j]);
                    pixels[u + i][v + j][1] = (double)(int)(pixels[u + i][v + j][1] / QC[i][j]);
                    pixels[u + i][v + j][2] = (double)(int)(pixels[u + i][v + j][2] / QC[i][j]);
                }
            }
        }
    }
    return;
}

void zigzag(vector<vector<vector<double>>>& pixels, vector<int>& zigzaged) {
    //Zigzag every 8*8 block
    for (int u = 0; u < SIZE; u += 8) {
        for (int v = 0; v < SIZE; v += 8) {
            bool rev = false;
            //traversal left-upside matrix
            for (int i = 0; i < 8; i++) {
                int r = i;
                int c = 0;
                int start = zigzaged.size();
                while (r >= 0 && c < 8) {
                    zigzaged.push_back(pixels[u + r][v + c][0]);
                    zigzaged.push_back(pixels[u + r][v + c][1]);
                    zigzaged.push_back(pixels[u + r][v + c][2]);
                    r--; c++;
                }
                if (rev) reverse(zigzaged.begin() + start, zigzaged.end());
                rev = !rev;
            }
            //traversal right-downside matrix
            for (int i = 1; i < 8; i++) {
                int r = 7;
                int c = i;
                int start = zigzaged.size();
                while (r >= 0 && c < 8) {
                    zigzaged.push_back(pixels[u + r][v + c][0]);
                    zigzaged.push_back(pixels[u + r][v + c][1]);
                    zigzaged.push_back(pixels[u + r][v + c][2]);
                    r--; c++;
                }
                if (rev) reverse(zigzaged.begin() + start, zigzaged.end());
                rev = !rev;
            }
        }
    }
    
    return;
}

void rlEncoding(vector<int>& zigzaged, vector<vector<int>>& rlCode) {
    for (int i = 0; i < zigzaged.size(); i += 8 * 8 * 3) {
        for (int k = 0; k < 3; k++) {
            int j = k;
            int cnt = 0;
            for (; j < 8 * 8 * 3; j += 3) {
                if (j < 3) {
                    rlCode.push_back({ zigzaged[i + j] });
                    continue;
                }
                if (cnt == 15) {
                    rlCode.push_back({ 15,zigzaged[i + j] });
                    cnt = 0;
                    continue;
                }
                if (zigzaged[i + j] == 0) cnt++;
                else {
                    rlCode.push_back({ cnt,zigzaged[i + j] });
                    cnt = 0;
                }
            }
            while (rlCode.back().size() > 1 && rlCode.back()[1] == 0) rlCode.pop_back();
            rlCode.push_back({ 0,0 });
        }
    }
    return;
}

inline int cntbit(int x) {
    //count how many bits in integer x.
    x = abs(x);
    int res = 0;
    while (x > 0) {
        res++;
        x >>= 1;
    }
    return res;
}
string getCode(int x) {
    //get the variable code. return the code as string
    if (x > 0) {
        string res;
        while (x > 0) {
            res += to_string(x & 1);
            x >>= 1;
        }
        reverse(res.begin(), res.end());
        return res;
    };
    x = abs(x);
    int bit = cntbit(x);
    int base = 1 << (bit - 1);
    int y=base - (x - base + 1);
    string res;
    while (y > 0) {
        res += to_string(y & 1);
        y >>= 1;
    }
    while (res.length() < bit) res += "0";
    reverse(res.begin(), res.end());
    return res;
}
void vlEncoding(vector<int> vec, vector<vector<int>>& vlCode, vector<string>& str) {
    //Input run-length code
    //Save (x,y) in vlcode; save codestring in str.
    //e.g. input {0,31} -> vlcode.push_back({0,5}); str.push_back("11111")
    if (vec.size() == 1) {
        int x = vec[0];
        vlCode.push_back({ cntbit(x) });
        str.push_back(getCode(x));
    }
    else {
        int x = vec[1];
        vlCode.push_back({ vec[0],cntbit(x) });
        str.push_back(getCode(x));
    }
    return;
}

/*
unordered_map<int, int> huffmanEncoding(vector<vector<int>> vec) {
    //Input data to be encoded, and return each's code length in huffman encoding rules.
    unordered_map<int, int> freq;
    for (vector<int> i : vec) {
        if (i.size() == 1) freq[i[0]]++;
        else freq[i[0] * 16 + i[1]]++;
    }
    struct TreeNode {
        int key;
        int freq;
        TreeNode* left;
        TreeNode* right;
        TreeNode(int k, int v) :key(k), freq(v), left(nullptr), right(nullptr) {};
        TreeNode(int k, int v, TreeNode* l, TreeNode* r) :key(k), freq(v), left(l), right(r) {};
    };
    struct cmp
    {
        bool operator()(TreeNode* a, TreeNode* b)
        {
            if (a->freq == b->freq) return a->key > b->key;
            else return a->freq >= b->freq;
        }
    };
    priority_queue < TreeNode*, vector<TreeNode*>, cmp> pq;
    for (auto i : freq) {
        TreeNode* newnode = new TreeNode(i.first, i.second);
        pq.push(newnode);
    }
    while (pq.size() > 1) {
        TreeNode* a = pq.top();
        pq.pop();
        TreeNode* b = pq.top();
        pq.pop();
        TreeNode* newnode = new TreeNode(-1, a->freq + b->freq, a, b);
        pq.push(newnode);
    }
    TreeNode* root = pq.top();
    unordered_map<int, int> res;
    queue<TreeNode* > q;
    q.push(root);
    int len = 1;
    while (!q.empty()) {
        for (int i = q.size(); i > 0; i--) {
            TreeNode* cur = q.front();
            q.pop();
            if (cur->key != -1) res[cur->key] = len;
            if (cur->left) q.push(cur->left);
            if (cur->right) q.push(cur->right);
        }
        len++;
    }
    cout << res.size();
    return res;
}
*/

void huffmanEncoding(vector<vector<int>>& vlCode, vector<string>& codestr, string& body) {
    //Four huffman encoding table
    vector<string> Y_DC{ "00","010","011","100","101","110","1110","11110","111110","1111110","11111110","111111110" };
    vector<string> C_DC{ "00","01","10","110","1110","11110","111110","1111110","11111110","111111110","1111111110","11111111110" };
    vector<vector<string>> Y_AC{
        {"1010","00","01","100","1011","11010","1111000","11111000","1111110110","1111111110000010","1111111110000011"},
        {"null","1100","11011","1111001","111110110","11111110110","1111111110000100","1111111110000101","1111111110000110","1111111110000111","1111111110001000"},
        {"null","11100","11111001","1111110111","111111110100","1111111110001001","1111111110001010","1111111110001011","1111111110001100","1111111110001101","1111111110001110"},
        {"null","111010","111110111","111111110101","1111111110001111","1111111110010000","1111111110010001","1111111110010010","1111111110010011","1111111110010100","1111111110010101"},
        {"null","111011","1111111000","1111111110010110","1111111110010111","1111111110011000","1111111110011001","1111111110011010","1111111110011011","1111111110011100","1111111110011101"},
        {"null","1111010","11111110111","1111111110011110","1111111110011111","1111111110100000","1111111110100001","1111111110100010","1111111110100011","1111111110100100","1111111110100101"},
        {"null","1111011","111111110110","1111111110100110","1111111110100111","1111111110101000","1111111110101001","1111111110101010","1111111110101011","1111111110101100","1111111110101101"},
        {"null","11111010","111111110111","1111111110101110","1111111110101111","1111111110110000","1111111110110001","1111111110110010","1111111110110011","1111111110110100","1111111110110101"},
        {"null","111111000","111111111000000","1111111110110110","1111111110110111","1111111110111000","1111111110111001","1111111110111010","1111111110111011","1111111110111100","1111111110111101"},
        {"null","111111001","1111111110111110","1111111110111111","1111111111000000","1111111111000001","1111111111000010","1111111111000011","1111111111000100","1111111111000101","1111111111000110"},
        {"null","111111010","1111111111000111","1111111111001000","1111111111001001","1111111111001010","1111111111001011","1111111111001100","1111111111001101","1111111111001110","1111111111001111"},
        {"null","1111111001","1111111111010000","1111111111010001","1111111111010010","1111111111010011","1111111111010100","1111111111010101","1111111111010110","1111111111010111","1111111111011000"},
        {"null","1111111010","1111111111011001","1111111111011010","1111111111011011","1111111111011100","1111111111011101","1111111111011110","1111111111011111","1111111111100000","1111111111100001"},
        {"null","11111111000","1111111111100010","1111111111100011","1111111111100100","1111111111100101","1111111111100110","1111111111100111","1111111111101000","1111111111101001","1111111111101010"},
        {"null","1111111111101011","1111111111101100","1111111111101101","1111111111101110","1111111111101111","1111111111110000","1111111111110001","1111111111110010","1111111111110011","1111111111110100"},
        {"null","1111111111110101","1111111111110110","1111111111110111","1111111111111000","1111111111111001","1111111111111010","1111111111111011","1111111111111100","1111111111111101","1111111111111110"},
    };
    vector<vector<string>> C_AC{
        {"00","01","100","1010","11000","11001","111000","1111000","111110100","1111110110","111111110100"},
        {"null","1011","111001","11110110","111110101","11111110110","111111110101","1111111110001000","1111111110001001","1111111110001010","1111111110001011"},
        {"null","11010","11110111","1111110111","111111110110","111111111000010","1111111110001100","1111111110001101","1111111110001110","1111111110001111","1111111110010000"},
        {"null","11011","11111000","1111111000","111111110111","1111111110010001","1111111110010010","1111111110010011","1111111110010100","1111111110010101","1111111110010110"},
        {"null","111010","111110110","1111111110010111","1111111110011000","1111111110011001","1111111110011010","1111111110011011","1111111110011100","1111111110011101","1111111110011110"},
        {"null","111011","1111111001","1111111110011111","1111111110100000","1111111110100001","1111111110100010","1111111110100011","1111111110100100","1111111110100101","1111111110100110"},
        {"null","1111001","11111110111","1111111110100111","1111111110101000","1111111110101001","1111111110101010","1111111110101011","1111111110101100","1111111110101101","1111111110101110"},
        {"null","1111010","11111111000","1111111110101111","1111111110110000","1111111110110001","1111111110110010","1111111110110011","1111111110110100","1111111110110101","1111111110110110"},
        {"null","11111001","1111111110110111","1111111110111000","1111111110111001","1111111110111010","1111111110111011","1111111110111100","1111111110111101","1111111110111110","1111111110111111"},
        {"null","111110111","1111111111000000","1111111111000001","1111111111000010","1111111111000011","1111111111000100","1111111111000101","1111111111000110","1111111111000111","1111111111001000"},
        {"null","111111000","1111111111001001","1111111111001010","1111111111001011","1111111111001100","1111111111001101","1111111111001110","1111111111001111","1111111111010000","1111111111010001"},
        {"null","111111001","1111111111010010","1111111111010011","1111111111010100","1111111111010101","1111111111010110","1111111111010111","1111111111011000","1111111111011001","1111111111011010"},
        {"null","111111010","1111111111011011","1111111111011100","1111111111011101","1111111111011110","1111111111011111","1111111111100000","1111111111100001","1111111111100010","1111111111100011"},
        {"null","11111111001","1111111111100100","1111111111100101","1111111111100110","1111111111100111","1111111111101000","1111111111101001","1111111111101010","1111111111101011","1111111111101100"},
        {"null","11111111100000","1111111111101101","1111111111101110","1111111111101111","1111111111110000","1111111111110001","1111111111110010","1111111111110011","1111111111110100","1111111111110101"},
        {"null","111111111000011","1111111111110110","1111111111110111","1111111111111000","1111111111111001","1111111111111010","1111111111111011","1111111111111100","1111111111111101","1111111111111110"},
    };

    int cnt = 0;//record whether the data is Y or Cr/Cb 

    //traversal vlCode and start build JPEG body.
    for (int i = 0; i < vlCode.size(); i++) {
        vector<int>& cur = vlCode[i];
        if (cur.size() == 1) {
            if (cnt == 0) body += Y_DC[cur[0]];
            else body += C_DC[cur[0]];
            cnt = (cnt + 1) % 3;
        }
        else {
            if (cnt == 1) body += Y_AC[cur[0]][cur[1]];
            else body += C_AC[cur[0]][cur[1]];
        }
        body += codestr[i];
    }
}

int generate(string path, string& binary) {
    ofstream ofs(path, ios::out | ios::binary);
    if (!ofs.is_open()) return -1;//Check whether the path is valid. If not, the program will exit with code -1.
    vector<unsigned char> data;
    unsigned char SOI = 0xD8;
    unsigned char EOI = 0xD9;
    data.push_back(SOI);
    for (int i = 0; i < binary.size(); i += 8) {
        unsigned char cur = 0;
        string byte = binary.substr(i, 8);
        for (char i : byte) {
            cur <<= 1;
            cur |= i - '0';
        }
        data.push_back(cur);
    }
    data.push_back(EOI);
    for (unsigned char c : data) ofs.write((char*)&c, sizeof(unsigned char));
    ofs.close();
    return 0;
}

int main()
{
    //Three-dimension vector, for save the pixel matrix.
    vector<vector<vector<double>>> pixels(SIZE, vector<vector<double>>(SIZE, vector<double>(3,0))); 
    
    //Load data into memory.
    if (load("demo.bin", pixels) == -1) {
        cerr << "Failed to open file. Please check the path." << endl;
        return -1;
    }
    
    //Convert colorspace, then process data through DCT and quantify.
    colorSpaceConversion(pixels);
    DCT(pixels);
    /*
    for (int i = 0; i < SIZE; i += 8) {
        for (int j = 0; j < SIZE; j += 8) {
            cout << pixels[i][j][0] << "\t" << pixels[i][j][1] << "\t" << pixels[i][j][2] << "\t**STAR**" << endl;
            for (int u = 0; u < 8; u++) {
                for (int v = 0; v < 8; v++) {
                    if (u == 0 && v == 0) continue;
                    cout << pixels[i + u][j + v][0] << "\t" << pixels[i + u][j + v][1] << "\t" << pixels[i + u][j + v][2] << endl;
                }
            }
        }
    }
    */
    quantify(pixels);
    
    //Zigzag arrangement.
    vector<int> zigzaged;
    zigzag(pixels, zigzaged);

    //Differencialize.
    for (int i = zigzaged.size() - 8 * 8 * 3; i >= 8 * 8 * 3; i -= 8 * 8 * 3) {
        for(int j=i;j<i+3;j++) zigzaged[j] -= zigzaged[j - 8 * 8 * 3];
    }

    //Run-length code.
    vector<vector<int>> rlCode;
    rlEncoding(zigzaged, rlCode);

    //Variable-length code.
    vector<vector<int>> vlCode; //Save (x,y)
    vector<string> codestr; //Save binary code.
    for (vector<int> i : rlCode) vlEncoding(i, vlCode, codestr);

    //Huffman Encoding.
    /*
    unordered_map<int, int> origin2length = huffmanEncoding(vlCode);
    unordered_map<int, vector<int>> length2origin;
    for (auto i : origin2length) length2origin[i.second].push_back(i.first);
    */
    string body;//Save binary code in string.
    huffmanEncoding(vlCode, codestr, body);

    //Generate JPEG image.
    string outputPath = "test.jpg";
    if (generate(outputPath, body) == -1) {
        cerr << "Failed to generate. Please check the path." << endl;
        return -1;
    }
    else cout << "Generated " << outputPath << endl;
}

