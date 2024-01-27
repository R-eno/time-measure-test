#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/err.h>

//ファイル名をグローバル変数で定義する
#define FILE_NAME "128KB_test_log.csv"
#define LOG_FILE "/home/eno/time_measure/test-log/"
#define ENCRYPTO_LOG "/home/eno/time_measure/test-evlog/"
#define MAX_LINES 100       // 最大行数
#define LINE_LENGTH 200000  // 各行の最大長

// 関数の定義
void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

//暗号化
void encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext, unsigned char *tag, int *ciphertext_len) {
    EVP_CIPHER_CTX *ctx;

    int len;

    // コンテキストの作成と初期化
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    // 暗号化の初期化
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, key, iv))
        handleErrors();

    // プレーンテキストを提供し、暗号文を取得
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    *ciphertext_len = len;

    // 残りの暗号文を取得
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    *ciphertext_len += len;

    // 認証タグを生成
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
        handleErrors();

    // コンテキストを解放
    EVP_CIPHER_CTX_free(ctx);
}

// ファイルからデータを読み込む関数
int loadData(const char *filename, char **data, int max_lines) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("ファイルを開けません");
        return -1;
    }

    int lineCount = 0;
    while (fgets(data[lineCount], LINE_LENGTH, file) && lineCount < max_lines) {
        data[lineCount][strcspn(data[lineCount], "\n")] = 0; // 改行文字を除去
        lineCount++;
    }

    fclose(file);
    return lineCount;
}

int main() {
    char log_file[100] = LOG_FILE;
    char encrypto_log_file[100] = ENCRYPTO_LOG;

    strcat(log_file, FILE_NAME);
    strcat(encrypto_log_file, FILE_NAME);

    char *data[MAX_LINES];
    for (int i = 0; i < MAX_LINES; i++) {
        data[i] = malloc(LINE_LENGTH);
        if (data[i] == NULL) {
            perror("メモリ割り当てエラー");
            return -1;
        }
    }

    int lineCount = loadData(log_file, data, MAX_LINES);
    if (lineCount == -1) {
        // エラー処理...
        return -1;
    }

    // 暗号化処理...
    // ここに暗号化処理のコード...

    // メモリ解放
    for (int i = 0; i < MAX_LINES; i++) {
        free(data[i]);
    }

    return 0;
}

