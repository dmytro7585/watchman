# Watchman
Simple (one line) TTY time & date monitor

### Build & Run dependencies:
```
GCC
Make
``` 
### Build & Run instructions.

1. Cloning the repository:
```
git clone https://github.com/dmytro7585/watchman.git
cd watchman
```
2. Program assembly
```
make
```
3. Launching the program
```
./watchman
```
4. An example of a launch with options:
(This command will run the program and display the time in 12-hour format with AM/PM and seconds.)
```
./watchman -s -a
```

5. Cleaning
To remove the generated files (object files and executable files), use the clean command:
```
make clean
```


### Options:
```
- no options: displays only local time hours and minutes
-h - displays help for all options in English
-s - additionally displays seconds
-d - shows the date (year-month-day)
-t shows the time zone (can be done later when everything else is ready)
-m - writes the name of the month
-u - displays Greenwich Mean Time
-w - writes the name of the day of the week
-a - AM/PM mode
```