if [ "$#" -ne 2 ]; then
	echo "Incorrect arguments given. Give 2 arguments"
	exit 1
fi

directory=$1
permission=$2

if [ ! -d "$directory" ]; then
	echo "Incorrect directory"
	exit 1
fi

if [[ "$permission" != "r" && "$permission" != "w" && "$permission" != "x" ]]; then
	echo "Incorrect permission given. Please specify r, w or x."
	exit 1
fi

case "$permission" in
	r) chmod_flag="u+r";;
	w) chmod_flag="u+w";;
	x) chmod_flag="u+x";;
esac

find "$directory" -type f -name "*.txt" -exec chmod "$chmod_flag" {} +

echo "Permission: $permission applied to all .txt files in directory: $directory."

	

