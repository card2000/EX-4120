
// $Revision: 1.2 $
varying highp vec2 texcoord;
uniform sampler2D texture;

#define DIST 0.005

void main(void)
{
	// This is a bit rubbish because it doesn't do proper separable blur and
	// it doesn't take very many samples or weight them or anything
	mediump vec4 a, b, c, d;
	a = texture2D(texture, texcoord + vec2(+DIST, 0.0));
	b = texture2D(texture, texcoord + vec2(-DIST, 0.0));
	c = texture2D(texture, texcoord + vec2(0.0, +DIST));
	d = texture2D(texture, texcoord + vec2(0.0, -DIST));
	gl_FragColor = (texture2D(texture, texcoord) + a + b + c + d) / 5.0;
}
