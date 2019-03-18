
// $Revision: 1.2 $

attribute vec4 position;
uniform float t;
uniform float s;
mat4 rotate;
mat4 scale;

#define TILT 0.436332312999

void main(void)
{
	rotate = mat4(
		cos(t),  sin(t)*sin(TILT), sin(t)*cos(TILT), 0,
		   0,       cos(TILT),        -sin(TILT),    0,
		-sin(t), cos(t)*sin(TILT), cos(t)*cos(TILT), 0,
		   0,           0,                0,         1
		);

	scale = mat4(
		s,  0,  0,  0,
		0,  s,  0,  0,
		0,  0,  s,  0,
		0,  0,  0,  1
		);

	gl_Position = rotate * scale * position;
}
