/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/

#include <sofa/component/constraint/lagrangian/solver/ConstraintSolverImpl.h>
#include <sofa/simulation/PropagateEventVisitor.h>
#include <sofa/simulation/events/BuildConstraintSystemEndEvent.h>
#include <sofa/simulation/events/SolveConstraintSystemEndEvent.h>

namespace sofa::component::constraint::lagrangian::solver
{

ConstraintProblem::ConstraintProblem()
    : tolerance(0.00001), maxIterations(1000),
      dimension(0), problemId(0)
{
}

ConstraintProblem::~ConstraintProblem()
{
}

void ConstraintProblem::clear(int nbConstraints)
{
    dimension = nbConstraints;
    W.resize(nbConstraints, nbConstraints);
    dFree.resize(nbConstraints);
    f.resize(nbConstraints);

    static unsigned int counter = 0;
    problemId = ++counter;
}

unsigned int ConstraintProblem::getProblemId()
{
    return problemId;
}

ConstraintSolverImpl::ConstraintSolverImpl()
    : l_constraintCorrections(initLink("constraintCorrections", "List of constraint corrections handled by this constraint solver"))
{}

ConstraintSolverImpl::~ConstraintSolverImpl()
{}

void ConstraintSolverImpl::init()
{
    ConstraintSolver::init();

    // Prevents ConstraintCorrection accumulation due to multiple AnimationLoop initialization on dynamic components Add/Remove operations.
    clearConstraintCorrections();

    // add all BaseConstraintCorrection from this context to the list of links and register this solver
    for (const auto& constraintCorrection :
        getContext()->getObjects<core::behavior::BaseConstraintCorrection>(core::objectmodel::BaseContext::SearchDown))
    {
        l_constraintCorrections.add(constraintCorrection);
        constraintCorrection->addConstraintSolver(this);
    }
}

void ConstraintSolverImpl::cleanup()
{
    clearConstraintCorrections();
    ConstraintSolver::cleanup();
}

void ConstraintSolverImpl::removeConstraintCorrection(core::behavior::BaseConstraintCorrection* s)
{
    l_constraintCorrections.remove(s);
}

void ConstraintSolverImpl::postBuildSystem(const core::ConstraintParams* cParams)
{
    sofa::simulation::BuildConstraintSystemEndEvent evBegin;
    sofa::simulation::PropagateEventVisitor eventPropagation( cParams, &evBegin);
    eventPropagation.execute(this->getContext());
}


void ConstraintSolverImpl::postSolveSystem(const core::ConstraintParams* cParams)
{
    sofa::simulation::SolveConstraintSystemEndEvent evBegin;
    sofa::simulation::PropagateEventVisitor eventPropagation( cParams, &evBegin);
    eventPropagation.execute(this->getContext());
}

void ConstraintSolverImpl::clearConstraintCorrections()
{
    for (const auto& constraintCorrection : l_constraintCorrections)
    {
        constraintCorrection->removeConstraintSolver(this);
    }
    l_constraintCorrections.clear();
}


} //namespace sofa::component::constraint::lagrangian::solver
