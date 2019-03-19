/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/compiler/jit/build_xla_ops_pass.h"
#include "tensorflow/compiler/jit/clone_constants_for_better_clustering.h"
#include "tensorflow/compiler/jit/encapsulate_subgraphs_pass.h"
#include "tensorflow/compiler/jit/encapsulate_xla_computations_pass.h"
#include "tensorflow/compiler/jit/increase_dynamism_for_auto_jit_pass.h"
#include "tensorflow/compiler/jit/mark_for_compilation_pass.h"
#include "tensorflow/compiler/jit/partially_decluster_pass.h"
#include "tensorflow/core/common_runtime/optimization_registry.h"

namespace tensorflow {

// PRE_PLACEMENT passes:

// EncapsulateXlaComputationsPass rewrites computations generated by the
// xla.compile() Python code into XlaLaunch nodes.
REGISTER_OPTIMIZATION(OptimizationPassRegistry::PRE_PLACEMENT, 26,
                      EncapsulateXlaComputationsPass);

// from
// third_party/tensorflow/compiler/tf2xla/functionalize_control_flow_pass_registration.cc
// FunctionalizeControlFlowPass: 27
//
// This pass looks at the graph and all associated FunctionDefs, and turns
// traditional control flow structure (Switch/Merge/etc.) into functional
// control flow structure (XlaIf/XlaWhile). Following passes must
// handle those FunctionDef correctly.

// POST_REWRITE_FOR_EXEC passes that support auto-clustering to enable XLA:

REGISTER_OPTIMIZATION(OptimizationPassRegistry::POST_REWRITE_FOR_EXEC, 5,
                      CloneConstantsForBetterClusteringPass);

REGISTER_OPTIMIZATION(OptimizationPassRegistry::POST_REWRITE_FOR_EXEC, 10,
                      MarkForCompilationPass);

REGISTER_OPTIMIZATION(OptimizationPassRegistry::POST_REWRITE_FOR_EXEC, 20,
                      IncreaseDynamismForAutoJitPass);

REGISTER_OPTIMIZATION(OptimizationPassRegistry::POST_REWRITE_FOR_EXEC, 30,
                      PartiallyDeclusterPass);

// The EncapsulateSubgraphs pass must run after the MarkForCompilationPass. We
// also need to run it after the graph been rewritten to have _Send nodes added
// for fetches. Before the _Send nodes are added, fetch nodes are identified by
// name, and encapsulation might remove that node from the graph.
REGISTER_OPTIMIZATION(OptimizationPassRegistry::POST_REWRITE_FOR_EXEC, 40,
                      EncapsulateSubgraphsPass);

// Must run after EncapsulateSubgraphsPass.
REGISTER_OPTIMIZATION(OptimizationPassRegistry::POST_REWRITE_FOR_EXEC, 50,
                      BuildXlaOpsPass);

}  // namespace tensorflow
