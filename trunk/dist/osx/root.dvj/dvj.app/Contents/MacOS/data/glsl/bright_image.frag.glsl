uniform sampler2D tex;
uniform float brightnessScalar;

void main()
{
	vec4 texColor = texture2D(tex,gl_TexCoord[0].st);
	gl_FragColor = gl_Color * texColor * brightnessScalar;
	gl_FragColor.a = gl_Color.a;
}

