#test_log.csvファイルから各行を取得し、文字列型として１行に合成する
#得られた文字列のデータサイズを取得し、リスト形式で書き込む

import csv
import binascii

log_file_name = '102400B_test_log.csv'

data_size_list = []
row_count=0

def bond_deta():
    global data_size_list
    #test_log.csvファイルを開き、各行を読み込み、文字列型として１行に合成する
    with open(log_file_name, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            row_str = ','.join(row)

            #行数カウント
            global row_count
            row_count+=1
        
            #row_strのデータサイズを取得し、リスト型の変数に書き込む
            data_size = len(row_str.encode('utf-8'))
            data_size_list.append(data_size)

            print(f"{row_count}行目： {data_size}byte")


if __name__ == '__main__':
    bond_deta()
    #data_size_listの要素数と各要素の合計値を取得する
    x=sum(data_size_list)/len(data_size_list)
    print(f"各行のデータサイズの平均：{x}byte")