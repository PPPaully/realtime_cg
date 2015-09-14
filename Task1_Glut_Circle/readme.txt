To : nuttapong26@gmail.com, nuttapon.vana@gmail.com
Subject : 2110594 HW2

By :

5631067121 Pittayathorn Nomrak  
5631073921 Panusorn Wonganankit 
5631076821 Mongkud Klungpornkun 
5631092821 Seehait Chockthanyawat

Extra credit :

• Anisotropic diffuse or specular shading
• Writing the image to a image file instead of to the screen.
• Some sort of "toon" shading.
• Other shapes besides spheres.
• Multiple spheres.

Additional command line option (Parenthesis is optional) : 
-co
This changes the rendered object(s) into cone shape. By default (without -co command), they will be sphere shape.
-an
This changes rendering type into anisotropic model shading. By default (without -an command), it will be simple model shading.
-no n
This creates and renders n object(s) into their own fixed position in their suitable integer-square table. The origin is still at the center of the area, and the half of maximum between object's width and height (or the radius for the sphere) still defines one unit of length. By default (without -no command), this will render only 1 object. 
-ts (n)
This renders the object(s) with toon shading by n shade(s). By default, the number of shade is 3 and minimum is 2.
-bmp w h
This causes no OpenGL window and renders all shaded object(s) into an image file with w-pixel width and h-pixel height in BMP format. Minimum width and height are 400 pixels.
-ppm w h
Same as -bmp command but in PPM format. If you add both -bmp and -ppm commands, it will write only BMP image file.