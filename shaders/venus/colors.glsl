vec3 ve_palette( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d )
{
    return a + b*cos( 6.283185*(c*t+d) );
}

vec3 ve_palette_0(in float t) 
{
    return ve_palette(t, vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5), 	vec3(1.0, 1.0, 1.0),	vec3(0.00, 0.33, 0.67));
    }

vec3 ve_palette_1(in float t) 
{
    return ve_palette(t, vec3(0.5, 0.5, 0.5),		vec3(0.5, 0.5, 0.5), 	vec3(1.0, 1.0, 1.0),	vec3(0.00, 0.10, 0.20));
}

vec3 ve_palette_2(in float t) 
{
    return ve_palette(t, vec3(0.5, 0.5, 0.5),		vec3(0.5, 0.5, 0.5), 	vec3(1.0, 1.0, 1.0),	vec3(0.30, 0.20, 0.20));
    }

vec3 ve_palette_3(in float t) 
{
    return ve_palette(t, vec3(0.5, 0.5, 0.5),		vec3(0.5, 0.5, 0.5), 	vec3(1.0, 1.0, 0.5),	vec3(0.80, 0.90, 0.30));
    }

vec3 ve_palette_4(in float t) 
{
    return ve_palette(t, vec3(0.5, 0.5, 0.5),		vec3(0.5, 0.5, 0.5), 	vec3(1.0, 0.7, 0.4),	vec3(0.00, 0.15, 0.20));
    }

vec3 ve_palette_5(in float t) 
{
    return ve_palette(t, vec3(0.5, 0.5, 0.5),		vec3(0.5, 0.5, 0.5), 	vec3(2.0, 1.0, 0.0),	vec3(0.50, 0.20, 0.25));
    }

vec3 ve_palette_6(in float t) 
{
    return ve_palette(t, vec3(0.8, 0.5, 0.4),		vec3(0.2, 0.4, 0.2), 	vec3(2.0, 1.0, 1.0),	vec3(0.00, 0.25, 0.25));
    }
