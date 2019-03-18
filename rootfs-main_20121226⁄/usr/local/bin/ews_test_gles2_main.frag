
// $Revision: 1.3 $

uniform mediump vec4 color;
mediump float d;

void main(void)
{
	d = clamp(1.0-gl_FragCoord.z, 0.0, 1.0);
	gl_FragColor = vec4(d, 0.9*d, 0.9*d, 1.0) * color;
}
