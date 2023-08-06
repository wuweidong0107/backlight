backlight: backlight.c
	gcc $^ -o $@ -lm

clean:
	rm -f backlight