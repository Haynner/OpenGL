#version 330 core

uniform int myrenderStyle;
uniform mat4 myview_matrix;
uniform mat4 myprojection_matrix;
uniform mat3 mynormal_matrix;
uniform mat4 mymodel_matrix;

uniform vec4 mylightPosition;
uniform vec4 mylightColor;
uniform vec3 mylightDirection;
uniform int mylightType;
uniform sampler2D tex;
uniform sampler2D bump;

in vec4 vectex_to_fragment;
in vec3 normal_to_fragment;
in vec2 texture_to_fragment;
in vec3 tangent_to_fragment;


void main (void)
{   
	vec4 position_ = myview_matrix * mymodel_matrix * vectex_to_fragment;
	//vec4 position_ = myprojection_matrix * myview_matrix * mymodel_matrix * vectex_to_fragment;
	vec3 position = (position_.xyz / position_.w);


	vec4 lightpos_ = mylightPosition;
	//lightpos_ =  myview_matrix * mymodel_matrix * lightpos_;
	lightpos_ =  myview_matrix * lightpos_;
	vec3 lightpos = (lightpos_.xyz) / lightpos_.w;

	vec4 kd = vec4(1,1,1,0);
	

	vec3 normal = normalize(mynormal_matrix * normal_to_fragment);
	vec3 t = normalize(mynormal_matrix * tangent_to_fragment);
	vec3 b = normalize(cross(normal,t));
	mat3 in_m = mat3(t,b,normal);
	mat3 out_m = transpose(in_m);

	//vec3 reflected_ray = normalize( reflect(position-lightpos,normal));
	vec3 eye = vec3(0,0,0);

	//eye = out_m * eye;
	//lightpos = out_m * lightpos;

	if(dot(normal,eye-position) == 0)
		gl_FragColor = vec4(0,0,0,0);
	else
	{
		vec4 Color;
		if (myrenderStyle == 0) Color = vec4(1,0,0,0);
		if (myrenderStyle == 1) Color = vec4(0,0.7,0,0);
		if (myrenderStyle == 2)
		{
			//normal = normalize(2.0 * texture2D(bump, texture_to_fragment.st).rgb - 1.f);
			Color = texture2D(tex, texture_to_fragment.st);
		}
		vec3 effetLight;
		float I;
		if (mylightType == 0) 
			{
				effetLight = lightpos-position;
				I = max(dot(normal, normalize(effetLight)),0);

			}
		else if (mylightType == 1)
			{
				 effetLight = normalize(mynormal_matrix *mylightDirection);
				 I = max(dot(normal, normalize(effetLight)),0);
			}
		else if (mylightType == 2)
			{
				 effetLight = lightpos-position;
				 I = max(dot(normal, normalize(effetLight)),0) * max(dot(normalize(mynormal_matrix * mylightDirection), normalize(effetLight)),0);
			}

		vec3 reflected_ray = normalize( reflect(-effetLight,normal));

		//ambiant
		gl_FragColor = Color * 0.2;

		// diffusion
		gl_FragColor += Color * kd * I;

		// speculaire
		//gl_FragColor += Color * mylightColor * pow( max(dot(reflected_ray,normalize(eye-position)),0),60);
		
	}

	

}