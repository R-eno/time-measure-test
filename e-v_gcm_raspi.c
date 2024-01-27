#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <sys/time.h>

//ファイル名をグローバル変数で定義する
#define FILE_NAME "128KB_test_log.csv"
#define LOG_FILE "/home/eno/time_measure/test-log/"
#define ENCRYPTO_LOG "/home/eno/time_measure/test-evlog/"

#define COUNT 100       // 回す回数
#define MAX_LINES 100       // 最大行数
#define LINE_LENGTH 2000000  // 各行の最大長

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



int decrypt_and_verify(unsigned char *ciphertext, int ciphertext_len, unsigned char *tag, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    int ret;

    // コンテキストの作成と初期化
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    // 復号の初期化
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, key, iv))
        handleErrors();

    // 暗号文を提供し、プレーンテキストを取得
    if(!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    // 認証タグを設定
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
        handleErrors();

    // 復号を完了
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    // コンテキストを解放
    EVP_CIPHER_CTX_free(ctx);

    if(ret > 0) {
        // 復号が成功
        plaintext_len += len;
        return plaintext_len;
    } else {
        // 復号に失敗
        return -1;
    }
}

int main() {
    //カレントディレクトリを抽出
    /*
    char current_dir[256];
    if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
        printf("current_dir: %s\n", current_dir);
    } else {
        perror("getcwd error");
        return -1;
    }
    */
    char log_file[100] = LOG_FILE;
    char encrypto_log_file[100] = ENCRYPTO_LOG;

    strcat(log_file, FILE_NAME);
    strcat(encrypto_log_file, FILE_NAME);

    //printf("%s\n", log_file);
    //printf("%s\n", encrypto_log_file);

    FILE *file = fopen(log_file, "r");
    if (file == NULL) {
        perror("log_fileファイルを開けません");
        return -1;
    }

    char *data[MAX_LINES]; // 各行のデータを格納するポインタの配列
    for (int i = 0; i < MAX_LINES; i++) {
        data[i] = malloc(LINE_LENGTH); // 各行のメモリを動的に割り当て
        if (data[i] == NULL) {
            perror("メモリ割り当てエラー");
            // エラー時のメモリ解放とプログラム終了
            for (int j = 0; j < i; j++) {
                free(data[j]);
            }
            fclose(file);
            return -1;
        }
    }

    int lineCount = 0;
    while (fgets(data[lineCount], LINE_LENGTH, file) && lineCount < MAX_LINES) {
        data[lineCount][strcspn(data[lineCount], "\n")] = 0; // 改行文字を除去
        lineCount++;
    }

    fclose(file);

    /*
    // 読み込んだデータの確認
    for (int i = 0; i < lineCount; i++) {
        printf("%s\n", data[i]);
        free(data[i]); // 不要になったメモリを解放
    }
    */

    // AES-GCMのキーとNonceの設定
    unsigned char key[] = "d4842a2380b0bf6a0323fde0629605c2";  // 32バイトのキー
    unsigned char nonce[] = "42a8413b2aeb5a582bb3945c";          // 適切な長さのNonce

    // 暗号化されたデータを格納する配列
    unsigned char encrypto_data[LINE_LENGTH];


    //時刻を取得
    struct timeval  tv1, tv2;

    //最初の1回は暗号化に時間がかかるため，のちに100回回して平均を取る    
    for (int i = 0; i < lineCount; i++)
    {

        unsigned char ciphertext[128];
        unsigned char tag[16];
        int ciphertext_len;
        encrypt((unsigned char *)data[i], strlen(data[i]), key, nonce, encrypto_data, tag, &ciphertext_len);

    }

    //のちの100回回して平均を取る
    gettimeofday(&tv1, NULL);
    for (int j=0;j<100;j++){
        
        for (int i = 0; i < lineCount; i++)
        {

            unsigned char ciphertext[128];
            unsigned char tag[16];
            int ciphertext_len;
            encrypt((unsigned char *)data[i], strlen(data[i]), key, nonce, encrypto_data, tag, &ciphertext_len);

        }

    }
    gettimeofday(&tv2, NULL);

    //時刻を比較して処理時間を出力
    printf("%f\n", (double)(tv2.tv_sec - tv1.tv_sec) + (double)(tv2.tv_usec - tv1.tv_usec) / 1000);

     
    



    

    
    
    // for (int i = 0; i < lineCount; i++) {

    //     unsigned char ciphertext[128];
    //     unsigned char tag[16];
    //     int ciphertext_len;
    //     encrypt((unsigned char *)data[i], strlen(data[i]), key, nonce, encrypto_data, tag, &ciphertext_len);


    //     //暗号データを1列目，ciphertext_lenを2列目，認証タグを3列目としてencrypto_log_fileへ格納する
    //     //暗号データと認証タグは16進数で出力する
    //     FILE *encrypto_file = fopen(encrypto_log_file, "a");
    //     if (encrypto_file == NULL) {
    //         perror("encrypto_log_fileファイルを開けません");
    //         return -1;
    //     }
    //     //暗号データを16進数に変換して書き込み
    //     for (int i = 0; i < ciphertext_len; i++) {
    //         fprintf(encrypto_file, "%02x", encrypto_data[i]);
    //     }

    //     //ciphertext_lenを書き込み
    //     fprintf(encrypto_file, ",%d,", ciphertext_len);

    //     //認証タグを１６進数に変換して書き込み
    //     for (int i = 0; i < 16; i++) {
    //         fprintf(encrypto_file, "%02x", tag[i]);
    //     }
    //     fprintf(encrypto_file, "\n");

    //     fclose(encrypto_file);



    // }





    return 0;
}