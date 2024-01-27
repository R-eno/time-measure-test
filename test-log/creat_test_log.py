#test_log.csvを開いて、1行あたり10列のランダムなデータを","を区切り文字として書き込む
#このとき1から5列目までは数値データ、5から10列目までは文字列データとする
#また１行辺りのデータサイズは1024バイトとして、100行生成する

import csv
import random
import string

NUM = 100 #生成する行数
SIZE = 512 #各行のデータサイズ（バイト）
NUM_COL = 10 #生成する列数
NUM_NUM = 5 #数値データの列数
NUM_STR = 5 #文字列データの列数

#NUM行NUM_COL列のランダムなデータを生成する
def create_random_data():
    global NUM, SIZE, NUM_COL, NUM_NUM, NUM_STR
    #NUM行NUM_COL列のランダムなデータを生成する
    for i in range(NUM):
        #1から5列目までは数値データ、5から10列目までは文字列データとする
        #1から5列目までの数値データを生成する
        num_list = []
        for j in range(NUM_NUM):
            num_list.append(random.randint(1, 100))
        num_size=len(str(num_list).encode('utf-8'))
        #5から10列目までの文字列データを生成する
        str_list = []
        for j in range(NUM_STR):
            str_list.append(''.join(random.choices(string.ascii_letters + string.digits, k=(SIZE-num_size)//NUM_STR)))
        #1から10列目までのデータを結合する
        data_list = num_list + str_list
        data_size = len(str(data_list).encode('utf-8'))

        #data_listを","を区切り文字として1行に書き込む
        file_name=str(SIZE)+"B_test_log.csv"
        with open(file_name, 'a') as f:
            writer = csv.writer(f)
            writer.writerow(data_list)

    print(data_list)
    print(f"最終行のデータサイズ：{data_size}byte")

if __name__ == '__main__':
    create_random_data()
    print("created test_log.csv")
