#!/bin/bash

# 定义目录路径
DIR1="data/output_data/twap_order"
DIR2="data/output_standard/twap_order"

# 维护相等数量
equal=0
total=0
# 遍历 DIR1 下的所有文件
for file in "$DIR1"/*; do
    # 获取文件的基本名（没有路径的部分）
    basefile=$(basename "$file")

    # 定义两个目录中的文件路径
    file1="$DIR1/$basefile"
    file2="$DIR2/$basefile"

    # 如果在 DIR2 中存在同名文件，就比较它们
    if [[ -f "$file2" ]]; then
        cmp -s "$file1" "$file2"
        status=$?

        # 根据 cmp 的结果显示消息
        if [[ $status -eq 0 ]]; then
            echo "✅ Files $basefile are identical."
            equal=$((equal+1))
        else
            echo "❌ Files $basefile are different."
        fi
        total=$((total+1))
    else
        echo "File $file2 does not exist."
    fi
done
echo "Pass: $equal/$total"

