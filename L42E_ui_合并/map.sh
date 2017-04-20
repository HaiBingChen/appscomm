#!/bin/sh

times=0

function scantxt_file(){
	
	#先cut掉前面没用的5行，在用\分割提取最后一段，然后用.bin分割提取第一段为名字
	#shell默认都是全局变量，要用局部变量要加local
	local file_name=$(tail -n +5 $1 | awk -F '\' '{print $NF}' | awk -F ".bin" '{print $1}')	

	#得到文件偏移，用空格做分割符，第4段为偏移量
	local file_offset=$(tail -n +5 $1 | awk '{print $4}')
	
	local file_name_arr=($file_name)
	local file_offset_arr=($file_offset)

	local i=0
	#for((i=0 ; i<${#file_name_arr[@]}; i++))
	while [ $i -lt ${#file_name_arr[@]} ] 
	do
		local filelist=${file_name_arr[$i]}
		local offset=${file_offset_arr[$i]}

		local file_tmp=$(find ${PWD} -name "$filelist.txt")	
		
		i=$[$i+1]

		if [ ! -z "$file_tmp" ] ; then
			
			#递归出栈后first_time的值为1说明是第一次进入的递归
			if [ $2 -eq 0 ] ; then
				echo -e "\n//===========================================================================//\n//$filelist"_base""
				echo -e "\n//===========================================================================//\n//$filelist"_base"" >>  L42E_map.h
			
				echo -e "#define $filelist"_base" $offset\n"
				echo -e "#define $filelist"_base" $offset\n" >> L42E_map.h
				
			else
				echo -e "\n//$filelist"_base""
				echo -e "\n//$filelist"_base"" >>  L42E_map.h
	
				echo -e "#define $filelist"_base" ($3 + $offset)\n"
				echo -e "#define $filelist"_base" ($3 + $offset)\n" >> L42E_map.h

			fi

			#1未下一个txt的名字,2为记录递归的次数,3为本次递归的偏移地址，在下次递归要用到
			scantxt_file $file_tmp $[$times+1] $filelist"_base"

		else	
			# $3为上次递归的偏移地址
			echo "#define $filelist ($3 + $offset)"
			echo "#define $filelist ($3 + $offset)" >> L42E_map.h

		fi

	done

	return 
}


if [ ! -n "$1" ] ; then
	echo "please input a txt file"
	exit -1
fi

#if [ -e L42E_map.h ] ; then
	rm L42E_map.h
#fi
  
touch L42E_map.h

scantxt_file $1 $times

