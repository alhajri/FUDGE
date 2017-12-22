/*
# <<BEGIN-copyright>>
# Copyright (c) 2016, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
# Written by the LLNL Nuclear Data and Theory group
#         (email: mattoon1@llnl.gov)
# LLNL-CODE-683960.
# All rights reserved.
# 
# This file is part of the FUDGE package (For Updating Data and 
#         Generating Evaluations)
# 
# When citing FUDGE, please use the following reference:
#   C.M. Mattoon, B.R. Beck, N.R. Patel, N.C. Summers, G.W. Hedstrom, D.A. Brown, "Generalized Nuclear Data: A New Structure (with Supporting Infrastructure) for Handling Nuclear Data", Nuclear Data Sheets, Volume 113, Issue 12, December 2012, Pages 3145-3171, ISSN 0090-3752, http://dx.doi.org/10. 1016/j.nds.2012.11.008
# 
# 
#     Please also read this link - Our Notice and Modified BSD License
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the disclaimer below.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the disclaimer (as noted below) in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of LLNS/LLNL nor the names of its contributors may be used
#       to endorse or promote products derived from this software without specific
#       prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC,
# THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 
# Additional BSD Notice
# 
# 1. This notice is required to be provided under our contract with the U.S.
# Department of Energy (DOE). This work was produced at Lawrence Livermore
# National Laboratory under Contract No. DE-AC52-07NA27344 with the DOE.
# 
# 2. Neither the United States Government nor Lawrence Livermore National Security,
# LLC nor any of their employees, makes any warranty, express or implied, or assumes
# any liability or responsibility for the accuracy, completeness, or usefulness of any
# information, apparatus, product, or process disclosed, or represents that its use
# would not infringe privately-owned rights.
# 
# 3. Also, reference herein to any specific commercial products, process, or services
# by trade name, trademark, manufacturer or otherwise does not necessarily constitute
# or imply its endorsement, recommendation, or favoring by the United States Government
# or Lawrence Livermore National Security, LLC. The views and opinions of authors expressed
# herein do not necessarily state or reflect those of the United States Government or
# Lawrence Livermore National Security, LLC, and shall not be used for advertising or
# product endorsement purposes.
# 
# <<END-copyright>>
*/

double xys1_answer[] = { /* Good to 1e-2. */
   1.000000e+02,  1.1231e-58,   1.004395e+02,  1.5208e-58,   1.008789e+02,  2.0552e-58,   1.013184e+02,  2.7719e-58,   1.017578e+02,  3.7313e-58,
   1.021973e+02,  5.0131e-58,   1.026367e+02,  6.7223e-58,   1.030762e+02,  8.9973e-58,   1.035156e+02,  1.2020e-57,   1.039551e+02,  1.6028e-57,
   1.043945e+02,  2.1334e-57,   1.048340e+02,  2.8345e-57,   1.052734e+02,  3.7592e-57,   1.057129e+02,  4.9768e-57,   1.061523e+02,  6.5773e-57,
   1.065918e+02,  8.6774e-57,   1.070312e+02,  1.1428e-56,   1.074707e+02,  1.5026e-56,   1.079102e+02,  1.9723e-56,   1.083496e+02,  2.5845e-56,
   1.087891e+02,  3.3811e-56,   1.092285e+02,  4.4161e-56,   1.096680e+02,  5.7587e-56,   1.101074e+02,  7.4973e-56,   1.105469e+02,  9.7455e-56,
   1.109863e+02,  1.2648e-55,   1.114258e+02,  1.6389e-55,   1.118652e+02,  2.1205e-55,   1.123047e+02,  2.7394e-55,   1.127441e+02,  3.5336e-55,
   1.131836e+02,  4.5512e-55,   1.136230e+02,  5.8533e-55,   1.140625e+02,  7.5169e-55,   1.145020e+02,  9.6393e-55,   1.149414e+02,  1.2343e-54,
   1.153809e+02,  1.5783e-54,   1.158203e+02,  2.0154e-54,   1.162598e+02,  2.5698e-54,   1.166992e+02,  3.2723e-54,   1.171387e+02,  4.1611e-54,
   1.175781e+02,  5.2841e-54,   1.180176e+02,  6.7013e-54,   1.184570e+02,  8.4872e-54,   1.188965e+02,  1.0735e-53,   1.193359e+02,  1.3560e-53,
   1.197754e+02,  1.7107e-53,   1.202148e+02,  2.1553e-53,   1.206543e+02,  2.7121e-53,   1.210938e+02,  3.4085e-53,   1.215332e+02,  4.2783e-53,
   1.219727e+02,  5.3634e-53,   1.224121e+02,  6.7156e-53,   1.228516e+02,  8.3984e-53,   1.232910e+02,  1.0490e-52,   1.237305e+02,  1.3087e-52,
   1.241699e+02,  1.6309e-52,   1.246094e+02,  2.0299e-52,   1.250488e+02,  2.5235e-52,   1.254883e+02,  3.1337e-52,   1.259277e+02,  3.8869e-52,
   1.263672e+02,  4.8158e-52,   1.268066e+02,  5.9599e-52,   1.272461e+02,  7.3677e-52,   1.276855e+02,  9.0979e-52,   1.281250e+02,  1.1222e-51,
   1.285645e+02,  1.3828e-51,   1.290039e+02,  1.7020e-51,   1.294434e+02,  2.0926e-51,   1.298828e+02,  2.5702e-51,   1.303223e+02,  3.1535e-51,
   1.307617e+02,  3.8652e-51,   1.312012e+02,  4.7326e-51,   1.316406e+02,  5.7888e-51,   1.320801e+02,  7.0735e-51,   1.325195e+02,  8.6346e-51,
   1.329590e+02,  1.0530e-50,   1.333984e+02,  1.2828e-50,   1.338379e+02,  1.5613e-50,   1.342773e+02,  1.8985e-50,   1.347168e+02,  2.3061e-50,
   1.351562e+02,  2.7987e-50,   1.355957e+02,  3.3932e-50,   1.360352e+02,  4.1102e-50,   1.364746e+02,  4.9741e-50,   1.369141e+02,  6.0139e-50,
   1.373535e+02,  7.2645e-50,   1.377930e+02,  8.7671e-50,   1.382324e+02,  1.0571e-49,   1.386719e+02,  1.2735e-49,   1.391113e+02,  1.5328e-49,
   1.395508e+02,  1.8432e-49,   1.399902e+02,  2.2146e-49,   1.404297e+02,  2.6586e-49,   1.408691e+02,  3.1887e-49,   1.413086e+02,  3.8214e-49,
   1.417480e+02,  4.5757e-49,   1.421875e+02,  5.4742e-49,   1.426270e+02,  6.5438e-49,   1.430664e+02,  7.8159e-49,   1.435059e+02,  9.3276e-49,
   1.439453e+02,  1.1123e-48,   1.443848e+02,  1.3252e-48,   1.448242e+02,  1.5777e-48,   1.452637e+02,  1.8768e-48,   1.457031e+02,  2.2308e-48,
   1.461426e+02,  2.6496e-48,   1.465820e+02,  3.1444e-48,   1.470215e+02,  3.7289e-48,   1.474609e+02,  4.4185e-48,   1.479004e+02,  5.2317e-48,
   1.483398e+02,  6.1899e-48,   1.487793e+02,  7.3182e-48,   1.492188e+02,  8.6456e-48,   1.496582e+02,  1.0206e-47,   1.500977e+02,  1.2040e-47,
   1.505371e+02,  1.4192e-47,   1.509766e+02,  1.6718e-47,   1.514160e+02,  1.9678e-47,   1.518555e+02,  2.3147e-47,   1.522949e+02,  2.7207e-47,
   1.527344e+02,  3.1957e-47,   1.531738e+02,  3.7511e-47,   1.536133e+02,  4.3999e-47,   1.540527e+02,  5.1574e-47,   1.544922e+02,  6.0412e-47,
   1.549316e+02,  7.0716e-47,   1.553711e+02,  8.2722e-47,   1.558105e+02,  9.6702e-47,   1.562500e+02,  1.1297e-46,   1.571289e+02,  1.5387e-46,
   1.580078e+02,  2.0904e-46,   1.588867e+02,  2.8326e-46,   1.597656e+02,  3.8286e-46,   1.606445e+02,  5.1620e-46,   1.615234e+02,  6.9428e-46,
   1.624023e+02,  9.3152e-46,   1.632812e+02,  1.2468e-45,   1.641602e+02,  1.6650e-45,   1.650391e+02,  2.2181e-45,   1.659180e+02,  2.9483e-45,
   1.667969e+02,  3.9100e-45,   1.676758e+02,  5.1738e-45,   1.685547e+02,  6.8309e-45,   1.694336e+02,  8.9993e-45,   1.703125e+02,  1.1830e-44,
   1.711914e+02,  1.5519e-44,   1.720703e+02,  2.0316e-44,   1.729492e+02,  2.6540e-44,   1.738281e+02,  3.4600e-44,   1.747070e+02,  4.5016e-44,
   1.755859e+02,  5.8453e-44,   1.764648e+02,  7.5751e-44,   1.773438e+02,  9.7977e-44,   1.782227e+02,  1.2648e-43,   1.791016e+02,  1.6297e-43,
   1.799805e+02,  2.0960e-43,   1.808594e+02,  2.6906e-43,   1.817383e+02,  3.4477e-43,   1.826172e+02,  4.4098e-43,   1.834961e+02,  5.6303e-43,
   1.843750e+02,  7.1760e-43,   1.852539e+02,  9.1302e-43,   1.861328e+02,  1.1597e-42,   1.870117e+02,  1.4704e-42,   1.878906e+02,  1.8613e-42,
   1.887695e+02,  2.3523e-42,   1.896484e+02,  2.9678e-42,   1.905273e+02,  3.7384e-42,   1.914062e+02,  4.7015e-42,   1.922852e+02,  5.9034e-42,
   1.931641e+02,  7.4009e-42,   1.940430e+02,  9.2639e-42,   1.949219e+02,  1.1578e-41,   1.958008e+02,  1.4449e-41,   1.966797e+02,  1.8004e-41,
   1.975586e+02,  2.2401e-41,   1.984375e+02,  2.7831e-41,   1.993164e+02,  3.4527e-41,   2.001953e+02,  4.2773e-41,   2.010742e+02,  5.2913e-41,
   2.019531e+02,  6.5366e-41,   2.028320e+02,  8.0638e-41,   2.037109e+02,  9.9341e-41,   2.045898e+02,  1.2222e-40,   2.054688e+02,  1.5016e-40,
   2.063477e+02,  1.8425e-40,   2.072266e+02,  2.2577e-40,   2.081055e+02,  2.7630e-40,   2.089844e+02,  3.3771e-40,   2.098633e+02,  4.1223e-40,
   2.107422e+02,  5.0257e-40,   2.116211e+02,  6.1195e-40,   2.125000e+02,  7.4421e-40,   2.133789e+02,  9.0396e-40,   2.142578e+02,  1.0967e-39,
   2.151367e+02,  1.3289e-39,   2.160156e+02,  1.6084e-39,   2.168945e+02,  1.9443e-39,   2.177734e+02,  2.3478e-39,   2.186523e+02,  2.8317e-39,
   2.195312e+02,  3.4114e-39,   2.204102e+02,  4.1053e-39,   2.212891e+02,  4.9348e-39,   2.221680e+02,  5.9254e-39,   2.230469e+02,  7.1070e-39,
   2.239258e+02,  8.5151e-39,   2.248047e+02,  1.0191e-38,   2.256836e+02,  1.2185e-38,   2.265625e+02,  1.4552e-38,   2.274414e+02,  1.7362e-38,
   2.283203e+02,  2.0694e-38,   2.291992e+02,  2.4639e-38,   2.300781e+02,  2.9307e-38,   2.309570e+02,  3.4824e-38,   2.318359e+02,  4.1340e-38,
   2.327148e+02,  4.9026e-38,   2.335938e+02,  5.8085e-38,   2.344727e+02,  6.8752e-38,   2.353516e+02,  8.1300e-38,   2.362305e+02,  9.6048e-38,
   2.371094e+02,  1.1336e-37,   2.379883e+02,  1.3368e-37,   2.388672e+02,  1.5749e-37,   2.397461e+02,  1.8538e-37,   2.406250e+02,  2.1800e-37,
   2.415039e+02,  2.5614e-37,   2.423828e+02,  3.0068e-37,   2.432617e+02,  3.5266e-37,   2.441406e+02,  4.1327e-37,   2.450195e+02,  4.8388e-37,
   2.458984e+02,  5.6607e-37,   2.467773e+02,  6.6165e-37,   2.476562e+02,  7.7273e-37,   2.494141e+02,  1.0514e-36,   2.511719e+02,  1.4258e-36,
   2.529297e+02,  1.9274e-36,   2.546875e+02,  2.5972e-36,   2.564453e+02,  3.4891e-36,   2.582031e+02,  4.6730e-36,   2.599609e+02,  6.2398e-36,
   2.617188e+02,  8.3075e-36,   2.634766e+02,  1.1029e-35,   2.652344e+02,  1.4599e-35,   2.669922e+02,  1.9272e-35,   2.687500e+02,  2.5371e-35,
   2.705078e+02,  3.3309e-35,   2.722656e+02,  4.3614e-35,   2.740234e+02,  5.6959e-35,   2.757812e+02,  7.4195e-35,   2.775391e+02,  9.6403e-35,
   2.792969e+02,  1.2495e-34,   2.810547e+02,  1.6154e-34,   2.828125e+02,  2.0835e-34,   2.845703e+02,  2.6809e-34,   2.863281e+02,  3.4414e-34,
   2.880859e+02,  4.4075e-34,   2.898438e+02,  5.6321e-34,   2.916016e+02,  7.1807e-34,   2.933594e+02,  9.1350e-34,   2.951172e+02,  1.1596e-33,
   2.968750e+02,  1.4688e-33,   2.986328e+02,  1.8566e-33,   3.003906e+02,  2.3420e-33,   3.021484e+02,  2.9481e-33,   3.039062e+02,  3.7037e-33,
   3.056641e+02,  4.6436e-33,   3.074219e+02,  5.8107e-33,   3.091797e+02,  7.2571e-33,   3.109375e+02,  9.0463e-33,   3.126953e+02,  1.1256e-32,
   3.144531e+02,  1.3978e-32,   3.162109e+02,  1.7328e-32,   3.179688e+02,  2.1442e-32,   3.197266e+02,  2.6486e-32,   3.214844e+02,  3.2659e-32,
   3.232422e+02,  4.0201e-32,   3.250000e+02,  4.9400e-32,   3.267578e+02,  6.0603e-32,   3.285156e+02,  7.4223e-32,   3.302734e+02,  9.0756e-32,
   3.320312e+02,  1.1079e-31,   3.337891e+02,  1.3504e-31,   3.355469e+02,  1.6433e-31,   3.373047e+02,  1.9967e-31,   3.390625e+02,  2.4223e-31,
   3.408203e+02,  2.9343e-31,   3.425781e+02,  3.5491e-31,   3.443359e+02,  4.2865e-31,   3.460938e+02,  5.1696e-31,   3.478516e+02,  6.2256e-31,
   3.496094e+02,  7.4868e-31,   3.513672e+02,  8.9909e-31,   3.531250e+02,  1.0782e-30,   3.548828e+02,  1.2913e-30,   3.566406e+02,  1.5444e-30,
   3.583984e+02,  1.8446e-30,   3.601562e+02,  2.2003e-30,   3.619141e+02,  2.6212e-30,   3.636719e+02,  3.1186e-30,   3.654297e+02,  3.7056e-30,
   3.671875e+02,  4.3977e-30,   3.689453e+02,  5.2126e-30,   3.707031e+02,  6.1710e-30,   3.724609e+02,  7.2967e-30,   3.742188e+02,  8.6175e-30,
   3.759766e+02,  1.0165e-29,   3.777344e+02,  1.1977e-29,   3.794922e+02,  1.4096e-29,   3.812500e+02,  1.6571e-29,   3.830078e+02,  1.9458e-29,
   3.847656e+02,  2.2823e-29,   3.865234e+02,  2.6740e-29,   3.882812e+02,  3.1295e-29,   3.917969e+02,  4.2729e-29,   3.953125e+02,  5.8096e-29,
   3.988281e+02,  7.8665e-29,   4.023438e+02,  1.0609e-28,   4.058594e+02,  1.4251e-28,   4.093750e+02,  1.9071e-28,   4.128906e+02,  2.5424e-28,
   4.164062e+02,  3.3769e-28,   4.199219e+02,  4.4691e-28,   4.234375e+02,  5.8938e-28,   4.269531e+02,  7.7458e-28,   4.304688e+02,  1.0145e-27,
   4.339844e+02,  1.3244e-27,   4.375000e+02,  1.7234e-27,   4.410156e+02,  2.2353e-27,   4.445312e+02,  2.8903e-27,   4.480469e+02,  3.7258e-27,
   4.515625e+02,  4.7884e-27,   4.550781e+02,  6.1361e-27,   4.585938e+02,  7.8403e-27,   4.621094e+02,  9.9895e-27,   4.656250e+02,  1.2693e-26,
   4.691406e+02,  1.6083e-26,   4.726562e+02,  2.0325e-26,   4.761719e+02,  2.5618e-26,   4.796875e+02,  3.2207e-26,   4.832031e+02,  4.0388e-26,
   4.867188e+02,  5.0522e-26,   4.902344e+02,  6.3044e-26,   4.937500e+02,  7.8482e-26,   4.972656e+02,  9.7470e-26,   5.007812e+02,  1.2077e-25,
   5.042969e+02,  1.4931e-25,   5.078125e+02,  1.8418e-25,   5.113281e+02,  2.2669e-25,   5.148438e+02,  2.7841e-25,   5.183594e+02,  3.4121e-25,
   5.218750e+02,  4.1731e-25,   5.253906e+02,  5.0934e-25,   5.289062e+02,  6.2041e-25,   5.324219e+02,  7.5420e-25,   5.359375e+02,  9.1507e-25,
   5.394531e+02,  1.1081e-24,   5.429688e+02,  1.3394e-24,   5.464844e+02,  1.6159e-24,   5.500000e+02,  1.9460e-24,   5.535156e+02,  2.3393e-24,
   5.570312e+02,  2.8071e-24,   5.605469e+02,  3.3626e-24,   5.640625e+02,  4.0212e-24,   5.675781e+02,  4.8007e-24,   5.710938e+02,  5.7218e-24,
   5.746094e+02,  6.8084e-24,   5.781250e+02,  8.0885e-24,   5.816406e+02,  9.5940e-24,   5.851562e+02,  1.1362e-23,   5.886719e+02,  1.3435e-23,
   5.921875e+02,  1.5863e-23,   5.957031e+02,  1.8701e-23,   5.992188e+02,  2.2014e-23,   6.027344e+02,  2.5878e-23,   6.062500e+02,  3.0376e-23,
   6.097656e+02,  3.5605e-23,   6.132812e+02,  4.1678e-23,   6.203125e+02,  5.6874e-23,   6.273438e+02,  7.7199e-23,   6.343750e+02,  1.0425e-22,
   6.414062e+02,  1.4008e-22,   6.484375e+02,  1.8730e-22,   6.554688e+02,  2.4925e-22,   6.625000e+02,  3.3017e-22,   6.695312e+02,  4.3540e-22,
   6.765625e+02,  5.7166e-22,   6.835938e+02,  7.4736e-22,   6.906250e+02,  9.7302e-22,   6.976562e+02,  1.2617e-21,   7.046875e+02,  1.6296e-21,
   7.117188e+02,  2.0966e-21,   7.187500e+02,  2.6874e-21,   7.257812e+02,  3.4320e-21,   7.328125e+02,  4.3673e-21,   7.398438e+02,  5.5382e-21,
   7.468750e+02,  6.9991e-21,   7.539062e+02,  8.8160e-21,   7.609375e+02,  1.1069e-20,   7.679688e+02,  1.3853e-20,   7.750000e+02,  1.7283e-20,
   7.820312e+02,  2.1498e-20,   7.890625e+02,  2.6662e-20,   7.960938e+02,  3.2970e-20,   8.031250e+02,  4.0655e-20,   8.101562e+02,  4.9993e-20,
   8.171875e+02,  6.1309e-20,   8.242188e+02,  7.4988e-20,   8.312500e+02,  9.1481e-20,   8.382812e+02,  1.1132e-19,   8.453125e+02,  1.3512e-19,
   8.523438e+02,  1.6361e-19,   8.593750e+02,  1.9764e-19,   8.664062e+02,  2.3818e-19,   8.734375e+02,  2.8639e-19,   8.804688e+02,  3.4358e-19,
   8.875000e+02,  4.1129e-19,   8.945312e+02,  4.9128e-19,   9.015625e+02,  5.8559e-19,   9.085938e+02,  6.9656e-19,   9.156250e+02,  8.2687e-19,
   9.226562e+02,  9.7960e-19,   9.296875e+02,  1.1583e-18,   9.367188e+02,  1.3669e-18,   9.437500e+02,  1.6101e-18,   9.507812e+02,  1.8931e-18,
   9.578125e+02,  2.2218e-18,   9.718750e+02,  3.0440e-18,   9.859375e+02,  4.1420e-18,   1.000000e+03,  5.5988e-18,   1.008789e+03,  6.7384e-18,
   1.017578e+03,  8.0902e-18,   1.026367e+03,  9.6900e-18,   1.035156e+03,  1.1579e-17,   1.043945e+03,  1.3805e-17,   1.052734e+03,  1.6421e-17,
   1.061523e+03,  1.9491e-17,   1.070312e+03,  2.3085e-17,   1.079102e+03,  2.7284e-17,   1.087891e+03,  3.2180e-17,   1.096680e+03,  3.7879e-17,
   1.105469e+03,  4.4498e-17,   1.123047e+03,  6.1055e-17,   1.140625e+03,  8.3149e-17,   1.158203e+03,  1.1243e-16,   1.175781e+03,  1.5098e-16,
   1.193359e+03,  2.0139e-16,   1.210938e+03,  2.6693e-16,   1.228516e+03,  3.5162e-16,   1.246094e+03,  4.6043e-16,   1.263672e+03,  5.9946e-16,
   1.281250e+03,  7.7615e-16,   1.298828e+03,  9.9955e-16,   1.316406e+03,  1.2806e-15,   1.333984e+03,  1.6325e-15,   1.351562e+03,  2.0711e-15,
   1.369141e+03,  2.6152e-15,   1.386719e+03,  3.2874e-15,   1.404297e+03,  4.1142e-15,   1.421875e+03,  5.1271e-15,   1.439453e+03,  6.3631e-15,
   1.457031e+03,  7.8656e-15,   1.474609e+03,  9.6853e-15,   1.492188e+03,  1.1881e-14,   1.509766e+03,  1.4522e-14,   1.527344e+03,  1.7687e-14,
   1.544922e+03,  2.1468e-14,   1.562500e+03,  2.5970e-14,   1.580078e+03,  3.1314e-14,   1.597656e+03,  3.7639e-14,   1.615234e+03,  4.5102e-14,
   1.632812e+03,  5.3884e-14,   1.650391e+03,  6.4190e-14,   1.667969e+03,  7.6250e-14,   1.685547e+03,  9.0327e-14,   1.703125e+03,  1.0672e-13,
   1.720703e+03,  1.2575e-13,   1.738281e+03,  1.4780e-13,   1.773438e+03,  2.0268e-13,   1.808594e+03,  2.7532e-13,   1.843750e+03,  3.7066e-13,
   1.878906e+03,  4.9474e-13,   1.914062e+03,  6.5500e-13,   1.949219e+03,  8.6043e-13,   1.984375e+03,  1.1219e-12,   2.019531e+03,  1.4525e-12,
   2.054688e+03,  1.8677e-12,   2.089844e+03,  2.3861e-12,   2.125000e+03,  3.0293e-12,   2.160156e+03,  3.8230e-12,   2.195312e+03,  4.7969e-12,
   2.230469e+03,  5.9860e-12,   2.265625e+03,  7.4302e-12,   2.300781e+03,  9.1761e-12,   2.335938e+03,  1.1277e-11,   2.371094e+03,  1.3793e-11,
   2.406250e+03,  1.6795e-11,   2.441406e+03,  2.0361e-11,   2.476562e+03,  2.4580e-11,   2.511719e+03,  2.9552e-11,   2.546875e+03,  3.5392e-11,
   2.582031e+03,  4.2224e-11,   2.617188e+03,  5.0192e-11,   2.652344e+03,  5.9453e-11,   2.687500e+03,  7.0182e-11,   2.757812e+03,  9.6843e-11,
   2.828125e+03,  1.3198e-10,   2.898438e+03,  1.7779e-10,   2.968750e+03,  2.3690e-10,   3.039062e+03,  3.1242e-10,   3.109375e+03,  4.0804e-10,
   3.179688e+03,  5.2807e-10,   3.250000e+03,  6.7752e-10,   3.320312e+03,  8.6217e-10,   3.390625e+03,  1.0887e-09,   3.460938e+03,  1.3646e-09,
   3.531250e+03,  1.6985e-09,   3.601562e+03,  2.1002e-09,   3.671875e+03,  2.5806e-09,   3.742188e+03,  3.1518e-09,   3.812500e+03,  3.8274e-09,
   3.882812e+03,  4.6225e-09,   3.953125e+03,  5.5537e-09,   4.023438e+03,  6.6393e-09,   4.093750e+03,  7.8993e-09,   4.234375e+03,  1.1032e-08,
   4.375000e+03,  1.5151e-08,   4.515625e+03,  2.0489e-08,   4.656250e+03,  2.7316e-08,   4.796875e+03,  3.5940e-08,   4.937500e+03,  4.6712e-08,
   5.078125e+03,  6.0027e-08,   5.218750e+03,  7.6325e-08,   5.359375e+03,  9.6095e-08,   5.500000e+03,  1.1987e-07,   5.640625e+03,  1.4825e-07,
   5.781250e+03,  1.8187e-07,   5.921875e+03,  2.2141e-07,   6.062500e+03,  2.6763e-07,   6.203125e+03,  3.2132e-07,   6.343750e+03,  3.8333e-07,
   6.625000e+03,  5.3600e-07,   6.906250e+03,  7.3347e-07,   7.187500e+03,  9.8441e-07,   7.468750e+03,  1.2982e-06,   7.750000e+03,  1.6851e-06,
   8.031250e+03,  2.1559e-06,   8.312500e+03,  2.7220e-06,   8.593750e+03,  3.3955e-06,   8.875000e+03,  4.1887e-06,   9.156250e+03,  5.1145e-06,
   9.437500e+03,  6.1863e-06,   1.000000e+04,  8.8216e-06,   1.031250e+04,  1.0615e-05,   1.062500e+04,  1.2664e-05,   1.125000e+04,  1.7602e-05,
   1.187500e+04,  2.3796e-05,   1.250000e+04,  3.1398e-05,   1.312500e+04,  4.0558e-05,   1.375000e+04,  5.1416e-05,   1.437500e+04,  6.4106e-05,
   1.500000e+04,  7.8750e-05,   1.625000e+04,  1.1433e-04,   1.750000e+04,  1.5893e-04,   1.875000e+04,  2.1313e-04,   2.000000e+04,  2.7734e-04,
   2.125000e+04,  3.5265e-04,   2.250000e+04,  4.3866e-04,   2.500000e+04,  6.4282e-04,   2.750000e+04,  8.8887e-04,   3.000000e+04,  1.1747e-03,
   3.500000e+04,  1.8636e-03,   4.000000e+04,  2.6785e-03,   5.000000e+04,  4.6105e-03,   6.000000e+04,  6.8050e-03,   7.000000e+04,  9.1418e-03,
   8.000000e+04,  1.1541e-02,   9.000000e+04,  1.3950e-02,   1.000000e+05,  1.6339e-02,   1.250000e+05,  2.2267e-02,   1.500000e+05,  2.7495e-02,
   2.000000e+05,  3.7113e-02,   2.500000e+05,  4.5358e-02 };