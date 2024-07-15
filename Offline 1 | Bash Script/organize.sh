#!/bin/bash

verbose=0
noexecute=0


if [[ "$#" -eq 5 && "$5" == "-noexecute" ]]
then
    noexecute=1
fi

if [[ "$#" -eq 5 && "$5" == "-v" ]]
then
    verbose=1
fi

if [ "$#" -eq 6 ]
then
    noexecute=1
    verbose=1

fi  

mkdir $2

echo student_id,type,matched,not_matched > $2/result.csv

for file in $1/* 
do
  temp=${file%.zip}
  unzip -q -j "$file" "*.c"  -d "$2/${temp: -7}"
done

if [ $verbose -eq 1 ];then
    echo Unzip done for C files successfully
fi

for file in $1/* 
do
  temp=${file%.zip}
unzip -q -j "$file" "*.java" -d "$2/${temp: -7}"
done

if [ $verbose -eq 1 ];then
    echo Unzip done for java files successfully
fi

for file in $1/* 
do
  temp=${file%.zip}
  unzip -q -j "$file" "*.py" -d "$2/${temp: -7}"
done

if [ $verbose -eq 1 ];then
    echo Unzip done for python files successfully
fi


if [ $verbose -eq 1 ];then
    echo Unzip done for all files successfully
fi

for file in $2/*
do
  if [ -e $file/*.c ]
  then
  	temp=`echo "$file" | cut -d "/" -f 2`
  	mkdir -p $2/C/$temp
  	mv $file/*.c $2/C/$temp/main.c	
    if [ $verbose -eq 1 ];then
        echo Organizing files of $temp
    fi

  	rm -r $file
  fi
done   

if [ $verbose -eq 1 ];then
    echo .c files stored in C folder
fi

for file in $2/*
do
  if [ -e $file/*.java ]
  then
  	temp=`echo "$file" | cut -d "/" -f 2`
  	mkdir -p $2/Java/$temp
  	mv $file/*.java $2/Java/$temp/Main.java	

    if [ $verbose -eq 1 ];then
        echo Organizing files of $temp
    fi

  	rm -r $file
  fi
done

if [ $verbose -eq 1 ];then
    echo .java files stored in Java folder
fi

for file in $2/*
do
  if [ -e $file/*.py ]
  then
  	temp=`echo "$file" | cut -d "/" -f 2`
  	mkdir -p $2/Python/$temp
  	mv $file/*.py $2/Python/$temp/main.py	

    if [ $verbose -eq 1 ];then
        echo Organizing files of $temp
    fi

  	rm -r $file
  fi
done

if [ $verbose -eq 1 ];then
    echo .py files stored in Python folder
fi

if [ $verbose -eq 1 ];then
   echo Task A done successfully
   echo All files are organized into directories named according to their student ID 
fi




# now task B

if [ $noexecute -eq 0 ]
then
	for file in $2/C/*
	do 
	    no_of_matches=0
	    temp=`echo "$file" | cut -d "/" -f 3`
	    `gcc $file/main.c -o $file/main.out`

        if [ $verbose -eq 1 ];then
            echo Executing files of $temp
        fi
        for((i=1;i<=3;i++))
        do
            `./$file/main.out < $3/test$i.txt > $file/out$i.txt`
            
            does_match=`diff $file/out$i.txt answers/ans$i.txt`
            
            if [ -z "$does_match" ]
            then
                no_of_matches=`expr $no_of_matches + 1`
            fi
	    done

	  didnt_match=`expr 3 - $no_of_matches`

	  echo $temp,C,$no_of_matches,$didnt_match >> $2/result.csv 
	done

fi

if [ $noexecute -eq 0 ]
then
    for file in $2/Java/*
	do 
	  no_of_matches=0
	  temp=`echo "$file" | cut -d "/" -f 3`
	  `javac $file/Main.java`

        if [ $verbose -eq 1 ];then
            echo Executing files of $temp
        fi
	  for ((i=1;i<=3;i++))
	  do
	    `java -cp $file Main < $3/test$i.txt > $file/out$i.txt`
	    does_match=`diff $file/out$i.txt answers/ans$i.txt`
	    
        

	    if [ -z "$does_match" ];then
		no_of_matches=`expr $no_of_matches + 1`
	    fi
	  done
	  didnt_match=`expr 3 - $no_of_matches`
	  echo $temp,Java,$no_of_matches,$didnt_match >> $2/result.csv 
	done
fi


if [ $noexecute -eq 0 ]
then
    for file in $2/Python/*
	do 
	  no_of_matches=0
	  temp=`echo "$file" | cut -d "/" -f 3`

        if [ $verbose -eq 1 ];then
            echo Executing files of $temp
        fi
	  for ((i=1;i<=3;i++))
	  do
	    `python3 ./$file/main.py < $3/test$i.txt > $file/out$i.txt`

	    does_match=`diff $file/out$i.txt answers/ans$i.txt`
	    
	    if [ -z "$does_match" ];then
		no_of_matches=`expr $no_of_matches + 1`

	    fi
	  done
	  didnt_match=`expr 3 - $no_of_matches`
	  echo $temp,Python,$no_of_matches,$didnt_match >> $2/result.csv 
	done
fi











