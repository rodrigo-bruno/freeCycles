for i in {1..10} ; do echo "i = $i" >> err; ./run.sh $i > log.$i 2>> err; done
