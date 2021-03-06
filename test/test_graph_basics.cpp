/******************************************************************************
** Copyright (c) 2015, Intel Corporation                                     **
** All rights reserved.                                                      **
**                                                                           **
** Redistribution and use in source and binary forms, with or without        **
** modification, are permitted provided that the following conditions        **
** are met:                                                                  **
** 1. Redistributions of source code must retain the above copyright         **
**    notice, this list of conditions and the following disclaimer.          **
** 2. Redistributions in binary form must reproduce the above copyright      **
**    notice, this list of conditions and the following disclaimer in the    **
**    documentation and/or other materials provided with the distribution.   **
** 3. Neither the name of the copyright holder nor the names of its          **
**    contributors may be used to endorse or promote products derived        **
**    from this software without specific prior written permission.          **
**                                                                           **
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       **
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         **
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     **
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      **
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    **
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  **
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    **
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    **
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      **
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        **
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* ******************************************************************************/
/* Narayanan Sundaram (Intel Corp.)
 * ******************************************************************************/

#include "catch.hpp"
#include "generator.h"
#include <algorithm>
#include <climits>
#include "GraphMatRuntime.h"

class custom_vertex_type {
  public: 
    int  iprop;
    float fprop;
  public:
    custom_vertex_type() {
      iprop = 0;
      fprop = 0.0f;
    }
  friend std::ostream &operator<<(std::ostream &outstream, const custom_vertex_type & val)
    {
      outstream << val.iprop << val.fprop; 
      return outstream;
    }
};



void test_graph_getset(int n) {
  auto E = generate_random_edgelist<int>(n, 16);
  GraphMat::Graph<custom_vertex_type> G;
  G.ReadEdgelist(E);
  E.clear();

  REQUIRE(G.getNumberOfVertices() == n);
  REQUIRE(G.nnz == n*16);

  for (int i = 1; i <= n; i++) {
    if (G.vertexNodeOwner(i)) {
      custom_vertex_type v;
      v.iprop = i;
      v.fprop = i*2.5;
      G.setVertexproperty(i, v);
    }
  }

  for (int i = 1; i <= n; i++) {
    if (G.vertexNodeOwner(i)) {
      REQUIRE(G.getVertexproperty(i).iprop == i);
      REQUIRE(G.getVertexproperty(i).fprop == Approx(2.5*i));
    }
  }

}

void test_graph_size(int n) {
  {
    auto E = generate_random_edgelist<int>(n, 16);
    GraphMat::Graph<custom_vertex_type> G;
    G.ReadEdgelist(E);
    E.clear();

    REQUIRE(G.getNumberOfVertices() == n);
    REQUIRE(G.nnz == n*16);
  }

  {
    auto E = generate_dense_edgelist<int>(n);
    GraphMat::Graph<custom_vertex_type> G;
    G.ReadEdgelist(E);
    E.clear();

    REQUIRE(G.getNumberOfVertices() == n);
    REQUIRE(G.nnz == n*n);
  }

  {
    auto E = generate_upper_triangular_edgelist<int>(n);
    GraphMat::Graph<custom_vertex_type> G;
    G.ReadEdgelist(E);
    E.clear();

    REQUIRE(G.getNumberOfVertices() == n);
    REQUIRE(G.nnz == n*(n-1)/2);
  }
}

void test_graph_io(int n) {
    auto E = generate_random_edgelist<int>(n, 16);
    GraphMat::random_edge_weights(&E, 128);

    GraphMat::edgelist_t<int> E1(E.m, E.n, E.nnz);
    memcpy(E1.edges, E.edges, E.nnz*sizeof(GraphMat::edge_t<int>));

    GraphMat::Graph<custom_vertex_type> G;
    G.ReadEdgelist(E);
    E.clear();
    
    GraphMat::edgelist_t<int> E2, E3, E4;
    G.getEdgelist(E2);

    collect_edges(E1, E3);
    std::sort(E3.edges, E3.edges + E3.nnz, edge_compare<int>);
    collect_edges(E2, E4);
    std::sort(E4.edges, E4.edges + E4.nnz, edge_compare<int>);
   
    REQUIRE(E3.nnz == E4.nnz); 
    for (int i = 0; i < E3.nnz; i++) {
      REQUIRE(E3.edges[i].src == E4.edges[i].src);
      REQUIRE(E3.edges[i].dst == E4.edges[i].dst);
      REQUIRE(E3.edges[i].val == E4.edges[i].val);
    }

    E1.clear();   
    E2.clear();   
    E3.clear();   
    E4.clear();   
}

TEST_CASE("Graph tests", "[random]")
{
  SECTION("test get set", "[get][set]") {
    test_graph_getset(500);
    test_graph_getset(1000);
  }
  SECTION("test size", "[size]") {
    test_graph_size(50);
    test_graph_size(500);
    test_graph_size(1000);
  }
  SECTION("test edgelist in/out", "[io]") {
    test_graph_io(500);
  }
}
