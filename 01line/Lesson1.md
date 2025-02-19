# Bresenham's Line Drawing Algoithm

## Why we need this?
Mathematically line is continuous which consists of inifinite points, however, as for the computers, the pixels are discrete. That is
to say we have to find a way to approximate continuous line with diecrete points.

## What is it?
In short, a simple algorithm to approximate line on screen. And in simple words, suppose we want to draw a line from (x_0, y_0) to (x_1, y_1), we need to know which pixels to be painted. Think it further, it simply decides whether we need to paint (x_k+1, y_k) or (x_k+1, y_k+1), to determine that, we apply a simple rule: rounding(四舍五入)， where the error is accumulated by extracting the pixel we traveled and the actual line traveld.
## How can we implement this(What did we do for this leeson?)

- First Attempt: naive method to draw the line, simply draw discrete pixels according to the slope.
- Second Attempt: Almost same core idea as Bresenham's algorithms, but limited to the order of points
- Third Attempt: Fix the previous problem by checking the value of x_0, x_1 and y_0, y_1 so that make sure the source is to the left bottom of the target
- Fourth Attempt: Polish the algorithm to reduce the number of division operations.
- Final Attempt:reduce the number of division operations even further