target = schedule_test
objects = schedule_test.o schedule.o
$(target): $(objects)
	g++ -o schedule_test $(objects)

$(objects): %.o:%.cpp
	g++ -c $? -o $@

.PHONY: clean
	
clean:
	rm schedule_test $(objects)
