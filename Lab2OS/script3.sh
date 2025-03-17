hour=$( date +"%H" )
user=$(whoami)
if [ "$hour" -ge 6 ] && [ "$hour" -le 12 ]; then
	echo "Good morning, $user !"
elif [ "$hour" -ge 12 ] && [ "$hour" -le 18 ]; then
	echo "Good afternoon, $user !"
elif [ "$hour" -ge 18 ] && [ "$hour" -le 22 ]; then
	echo "Good evening, $user !"
elif [ "$hour" -ge 22 ] && [ "$hour" -le 24 ] || [ "$hour" -ge 0 ] && [ "$hour" -lt 6 ]; then
	echo "Good night, $user !"
fi

