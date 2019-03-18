
// $Revision: 1.2 $
attribute vec4 position;
varying vec2 texcoord;
attribute vec2 inputtexcoord;

void main(void)
{
	texcoord = inputtexcoord;
	gl_Position = position;
}
