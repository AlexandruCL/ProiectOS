if [ "$#" -ne 3 ]; then
	echo "Incorrect number of arguments given."
	exit 1
fi

num1=$1
operator=$2
num2=$3

case "$operator" in 
	"+") result=$(echo "$num1 + $num2" | bc);;
	"-") result=$(echo "$num1 - $num2" | bc);;
	"*") result=$(echo "$num1 * $num2" | bc);;
	"/")
		if [ "$num2" = "0" ]; then
			echo "Cannot divide by 0"
			exit 1
		fi
		result=$(echo "$num1 / $num2" | bc);;
	*)
		echo "Please use a correct operator. + - * /"
		exit 1
		;;
esac

echo "$result"
	
