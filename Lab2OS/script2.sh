if [ "$#" -ne 3 ]; then
	echo "Incorrect number of arguments given."
	exit 1
fi

num1=$1
operator=$2
num2=$3

if [ "$operator" = "+" ]; 
then
	result=$(echo "$num1 + $num2" | bc)
elif [ "$operator" = "-" ]; 
then 
	result=$(echo "$num1 - $num2" | bc )
elif [ "$operator" = "*" ];
then
	result=&(echo "$num1 * $num2"| bc )
elif [ "$operator" = "/" ];
then
	if [ "$num2" = "0" ]; then
		echo "Cannot divide by 0"
		exit 1
	fi 
	result=$(echo "$num1 / $num2"| bc )
else
	echo "Incorrect operator given."
	exit 1
fi

echo "$result"
