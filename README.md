# hello_taskflow

To run:

1. Clone the `taskflow` and `toml11` c++ directories into the root folder
2. Build with: `g++ -std=c++20 simple.cpp -I taskflow/ -I toml11/ -O2 -pthread -o simple`
3. Run `/.simple`

You should see the following printed in your terminal:

```
Output from python/script1.py: script1

Output from python/script2.py: script2

Output from python/script3.py: script3
```
