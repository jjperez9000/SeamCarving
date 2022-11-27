# SeamCarving
Implementation of a seam carving algorithm on black and white images. 

Seam carving is removing the 'least interesting' column of pixels from an image. 
For black and white images this involves 'blurring' the image and then using a shorest paths algorithm to find the darkest path along the 
blurred imaged from the top to the bottom and then removing it. In the end you get the ability to squish images while still understanding what they
were originally supposed to look like. 
