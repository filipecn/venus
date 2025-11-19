#ifndef VENUS_VOLUME_GLSL
#define VENUS_VOLUME_GLSL

#include "debug.glsl"
#include "sampling.glsl"
#include "geometry.glsl"

// float sampleMediumDensity(vec3 wp);

struct Medium {
  vec3 sigma_a; // absorption coefficient
  vec3 sigma_s; // scattering coefficient
  vec3 Le;      // medium emission
};

struct PhaseFunctionSample {
  vec3  wi;  // scattering direction
  float p;   // phase function value
  float pdf; // pdf at sample
};

struct RayMajorantSegment {
  float t_min; // segment start
  float t_max; // segment end
  vec3 mu_maj; // region majorant coefficient
};

PhaseFunctionSample sample_pf_isotropic(inout uint seed) {
  PhaseFunctionSample pf;
  pf.wi = sampleUniformSphere(seed);
  pf.p = VENUS_1_4PI;
  pf.pdf = pf.p;
  return pf;
}

RayMajorantSegment nextMajorantSegment(in Medium medium, Ray ray) {
  clipRay(ray);
  RayMajorantSegment seg;
  seg.t_min = 0.0;
  seg.t_max = ray.t_max;
  seg.mu_maj = medium.sigma_a + medium.sigma_s;
  return seg;
}

// Solve RTE by Delta Tracking
vec3 solveRTE(in Medium medium, Ray ray, in uint max_depth, inout uint seed) {
  // RTE final value
  vec3 L = vec3(0.0);

  // Transmittance T_maj term
  float T_maj = 1.0;
  // Recursive accumulation term
  float beta = 0.0;
  // Recursion depth
  uint depth = 1;
  // loop iteration
  uint iterations = 0;

  // TODO: accept multiple segments
  RayMajorantSegment seg = nextMajorantSegment(medium, ray);

  // path integration
  while(true) {
    iterations++;
    if(iterations >3)
      break;


    // compute the new sample position
    // with free-path
    float free_path = -log(1.0 - rand(seed)) / seg.mu_maj[0];
    free_path = 0.5;
    float t = t_min + free_path;
    vec3 x = ray.o + ray.d * t;
    if(iterations == 1) {
      print_frag(free_path);
      print_frag(t);
      print_frag(x);
    }
    if(t < seg.t_max) {
      // we are inside majorant segment
	    T_maj *= exp(-(t-t_min) * seg.mu_maj[0]);

      // compute quantities
      // sample the volume at x
      float d_x = 100 * sampleMediumDensity(x);

      // define absorption and scattering terms
      float mu_a = clamp(d_x * medium.sigma_a[0], 0.0, seg.mu_maj[0]);
      float mu_s = clamp(d_x * medium.sigma_s[0], 0.0, seg.mu_maj[0]);

      // define interaction probabilities
      float P_a = mu_a / seg.mu_maj[0];
      float P_s = mu_s / seg.mu_maj[0];


      print_frag(121);
      print_frag(P_a);

      // Russian Roulette
	    float r = rand(seed);
	    if(r < P_a) { // absorption occurs
        print_frag(1.0);
		    L += beta * medium.Le;
        print_frag(beta);
        break;
	    } else if (r < P_a + P_s) {
		    // scattering occurs
	      if(depth++ > max_depth)
		      break;
        print_frag(2.0);
        clipRay(ray);
	      PhaseFunctionSample ps;
		    ps = sample_pf_isotropic(seed);
		    ray.d = ps.wi;
		    ray.o = x;
		    beta *= ps.p / ps.pdf;
        t_min = 0;
        continue;
	    } else { 
        // null-collision 
		    // we allow execution to continue
        t_min = t;
	    }
    } else {
      // we past majorant segment
      T_maj *= exp(-(seg.t_max - t_min) * seg.mu_maj[0]);
		  break;
    }
  }

  return L;
}

#endif
