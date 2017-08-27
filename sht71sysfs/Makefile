
.PHONY : clean install uninstall

sht71readout : main.cpp
	g++ --std=c++14 -Wall $^ -o $@

install: sht71readout
	cp $^ /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/sht71readout

clean:
	rm -f sht71readout
