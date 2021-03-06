#=========================================================================================================
#
#  gmr Script for Gmacs: Example for BBRKC Demonstration Model
#  Authors: Athol Whitten, Jim Ianelli
#  Info: https://github.com/seacode/gmr
#
#=========================================================================================================

# Load gmr package for Gmacs:
library(gmr)

# Set working directory to that containing Gmacs model results:
# setwd("c:/seacode/gmacs/examples/demo")
# setwd("~/_mymods/seacode/gmacs/examples/demo")
# setwd("~/_mymods/seacode/gmacs/examples/bbrkc")
setwd("/Users/stevenmartell1/Documents/CURRENT PROJECTS/GMACS/examples/bbrkc")

# Set theme for ggplot2 (works for themes classic, minimal, gray, bw):
set_ggtheme('bw')

# Read report file and create gmacs report object (a list):
gmrep <- read_admb('gmacs')

# Get plots of interest:
plot_catch(gmrep)
plot_growth(gmrep)
plot_catch(gmrep,plot_res=T)

plot_growth_inc(gmrep)
plot_cpue(gmrep)

plot_sizecomp(gmrep,which_plots=c(1))
plot_sizecomp(gmrep)
plot_sizecomp_res(gmrep, which_plots=c(1))


plot_sizecomp(gmrep,which_plots=c(11))
plot_sizecomp_res(gmrep)
plot_sizetransition(gmrep)

plot_selectivity(gmrep)
plot_recruitment(gmrep)
plot_datarange(gmrep)
plot_ssb(gmrep)
plot_naturalmortality(gmrep)

shiny_gmacs(gmrep)

#=========================================================================================================
