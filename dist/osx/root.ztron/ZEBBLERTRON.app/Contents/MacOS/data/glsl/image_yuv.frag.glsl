//image_yuv.frag.glsl

uniform sampler2D myTextureY;
uniform sampler2D myTextureU;
uniform sampler2D myTextureV;
uniform float brightnessScalar;

void main()
{
	float texColorY = texture2D(myTextureY,gl_TexCoord[0].st).r;
	float texColorU = texture2D(myTextureU,gl_TexCoord[1].st).r;
	float texColorV = texture2D(myTextureV,gl_TexCoord[2].st).r;

	//Next 6 lines from:
	//http://www.fourcc.org/source/YUV420P-OpenGL-GLSLang.c

	texColorY=1.1643*(texColorY-0.0625);
	texColorU=texColorU-0.5;
	texColorV=texColorV-0.5;

	gl_FragColor.r=texColorY+1.5958*texColorV;
	gl_FragColor.g=texColorY-0.39173*texColorU-0.81290*texColorV;
	gl_FragColor.b=texColorY+2.017*texColorU;

	gl_FragColor.r = gl_FragColor.r * gl_Color.r * brightnessScalar;
	gl_FragColor.g = gl_FragColor.g * gl_Color.g * brightnessScalar;
	gl_FragColor.b = gl_FragColor.b * gl_Color.b * brightnessScalar;
	gl_FragColor.a = gl_Color.a;
}

