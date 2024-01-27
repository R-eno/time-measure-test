#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <sys/time.h>


//ファイル名をグローバル変数で定義する
#define FILE_NAME "256B_test_log.csv"
#define LOG_FILE "/home/eno/time_measure/test-log/"
#define ENCRYPTO_LOG "/home/eno/time_measure/test-evlog/"

#define MAX_LINES 100       // 最大行数
#define LINE_LENGTH 2000000  // 各行の最大長

// 関数の定義
void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
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

    // AES-GCMのキーとNonceの設定
    unsigned char key[] = "d4842a2380b0bf6a0323fde0629605c2";  // 32バイトのキー
    unsigned char nonce[] = "42a8413b2aeb5a582bb3945c";          // 適切な長さのNonce

    
    // encrypto_log_fileファイルの読み込み
    //各行には,区切りで暗号データ，暗号データサイズ，タグが入っている
    FILE *fp;
    char data[LINE_LENGTH];
    char *datas[MAX_LINES];
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;

    fp = fopen(encrypto_log_file, "r");
    if (fp == NULL) {
        perror("log_fileファイルを開けません");
        return -1;
    }

    while (fgets(data, LINE_LENGTH, fp) != NULL) {
        datas[i] = malloc(LINE_LENGTH);
        if (datas[i] == NULL) {
            perror("メモリ割り当てエラー");
            // エラー時のメモリ解放とプログラム終了
            for (j = 0; j < i; j++) {
                free(datas[j]);
            }
            fclose(fp);
            return -1;
        }
        strcpy(datas[i], data);
        i++;
    }

    fclose(fp);
    
    // //読み込んだデータの確認
    // for (j = 0; j < i; j++) {
    //     printf("%s\n", datas[j]);
    // }


    //datasに格納された文字列1文字ずつ確認し，カンマがある場合はカンマの位置を記録
    int comma_pos[i][3];
    for (j = 0; j < i; j++) {
        for (k = 0; k < 3; k++) {
            comma_pos[j][k] = -1;
        }
    }
    for (j = 0; j < i; j++) {
        for (k = 0; k < strlen(datas[j]); k++) {
            if (datas[j][k] == ',') {
                comma_pos[j][l] = k;
                l++;
            }
        }
        l = 0;
    }
    //記録したカンマの位置をもとに，各行の暗号データ，暗号データサイズ，タグを取得
    unsigned char encrypto_data[i][LINE_LENGTH];
    int encrypto_data_len[i];
    unsigned char tag[i][16];

    for (j = 0; j < i; j++) {
        for (k = 0; k < comma_pos[j][0]; k++) {
            sscanf(datas[j] + k, "%02hhx", &encrypto_data[j][k]);
        }
        encrypto_data_len[j] = atoi(datas[j] + comma_pos[j][0] + 1);
        for (k = 0; k < comma_pos[j][2] - comma_pos[j][1] - 1; k++) {
            sscanf(datas[j] + comma_pos[j][1] + 1 + k, "%02hhx", &tag[j][k]);
        }
    }

    //取得した暗号データ，暗号データサイズ，タグを確認
    for (j = 0; j < i; j++) {
        for (k = 0; k < comma_pos[j][0]; k++) {
            printf("%02x", encrypto_data[j][k]);
        }
        printf("\n");
        printf("%d\n", encrypto_data_len[j]);
        for (k = 0; k < comma_pos[j][2] - comma_pos[j][1] - 1; k++) {
            printf("%02x", tag[j][k]);
        }
        printf("\n");
    }



    // char *token;
    // char *ptr;
    // unsigned char encrypto_data[LINE_LENGTH];
    // unsigned char tag[16];
    // int encrypto_data_len = 0;
    // unsigned char plaintext[LINE_LENGTH];
    // int plaintext_len = 0;
    // int ret = 0;

    // for (j = 0; j < i; j++) {
    //     token = strtok(datas[j], ",");
    //     k = 0;
    //     while (token != NULL) {
    //         switch (k) {
    //             case 0:
    //                 //printf("encrypto_data: %s\n", token);
    //                 //encrypto_data = token;
    //                 ptr = token;
    //                 for (l = 0; l < strlen(token); l++) {
    //                     sscanf(ptr, "%02hhx", &encrypto_data[l]);
    //                     ptr += 2;
    //                 }
    //                 break;
    //             case 1:
    //                 //printf("encrypto_data_len: %s\n", token);
    //                 encrypto_data_len = atoi(token);
    //                 //printf("encrypto_data_len: %d\n", encrypto_data_len);
    //                 break;
    //             case 2:
    //                 //printf("tag: %s\n", token);
    //                 ptr = token;
    //                 for (l = 0; l < strlen(token); l++) {
    //                     sscanf(ptr, "%02hhx", &tag[l]);
    //                     ptr += 2;
    //                 }
    //                 break;
    //             default:
    //                 //printf("default\n");
    //                 break;
    //         }
    //         token = strtok(NULL, ",");
    //         k++;
    //     }
    //     //printf("encrypto_data: %s\n", encrypto_data);
    //     //printf("encrypto_data_len: %d\n", encrypto_data_len);
    //     //printf("tag: %s\n", tag);

    //     //復号とタグの検証
    //     plaintext_len = decrypt_and_verify(encrypto_data, encrypto_data_len, tag, key, nonce, plaintext);
    //     if (plaintext_len == -1) {
    //         printf("Decryption failed\n");
    //     } else {
    //         plaintext[plaintext_len] = '\0';
    //         printf("Decryption succeeded\n");
    //         //printf("plaintext: %s\n", plaintext);
    //     }
    // }


    //メモリ解放
    for (j = 0; j < i; j++) {
        free(datas[j]);
    }

    return 0;
}