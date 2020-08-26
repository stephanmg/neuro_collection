# neuro_collection
UG4 plugin for unified neuro-related functionality

This plugin aims at collecting and unifying any neuro-related functionality that is developed for UG4 and that the author thinks of as usable for others.

## CI

[![Build Status (OSX/Linux)](https://travis-ci.org/NeuroBox3D/neuro_collection.svg?branch=master)](https://travis-ci.org/NeuroBox3D/neuro_collection)
[![Build status (Windows)](https://ci.appveyor.com/api/projects/status/di4nw042lggbyah8?svg=true)](https://ci.appveyor.com/project/stephanmg/neuro-collection)

Travis CI is used to trigger the build of downstream projects (Which depend on neuro_collection). See `.travis.yml` for details.
Currently two downstream projects are triggered (The latter project triggers another downstream project https://github.com/stephanmg/non-vr-grids): https://github.com/stephanmg/vr-grids and https://github.com/stephanmg/BranchGenerator
