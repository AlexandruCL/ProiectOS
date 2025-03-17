if [ "$#" -ne 3 ]; then
	echo "Incorrect arguments given. Give 3 arguments"
	exit 1
fi

num1=$1
num2=$2
num3=$3

largest=$num1

if [ "$num2" -gt "$largest" ]; then 
	largest=$num2
fi

if [ "$num3" -gt "$largest" ]; then 
        largest=$num3
fi

echo "$largest"
