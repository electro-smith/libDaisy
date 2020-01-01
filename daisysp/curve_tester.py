import math
import matplotlib.pyplot as plt

def triangle(size):
    shape = []
    for i in range(0, size):
        val = 1.0 - abs((-0.5 + (i/size)) * 2.0)
        shape.append(val)
    return shape

def transeg(inval, pos, size, curve):
    #beg = pos > size / 2 ? 1.0 : 0.0
    val = inval
    if pos > size / 2.0:
        beg = 1.0
    else:
        beg = 0.0

    out = beg + (val - beg) * (1 - math.exp(pos * curve / (size - 1))) / (1 - math.exp(curve))
    return out

test_size = 512
out_shape = list(range(test_size))
x_scale = list(range(test_size))
test_curve = 1.0
linear_shape = triangle(test_size)
for i in range(0, test_size):
    out_shape[i] = transeg(linear_shape[i], i, test_size, test_curve)
    x_scale[i] = i


plt.plot(x_scale,linear_shape, label="lin")
plt.plot(x_scale,out_shape, label="transeg")
plt.title('transeg test')
plt.legend() 
plt.show()



