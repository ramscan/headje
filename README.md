# headje
tail -f for head

I wrote this, while building a site using Laravel. I needed to see the contents of a class object. So, I dumped the contents using file_put_contents("/tmp/debug.txt",print_r($obj,true)). Why not use dd($obvj)? dd() halts execution of the program. Due to the amount of data being spit out, the part I was interested in would scroll off the screen when I used tail -f /tmp/debug.txt, and when I went looking for head -f, found no such option. So, this program was born.

A future version might allow cursor keys to scroll around in the output.
