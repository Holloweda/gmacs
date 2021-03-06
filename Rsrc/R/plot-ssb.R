#' Plot predicted spawning stock biomass (ssb)
#'
#' Spawning biomass may be defined as all males or some combination of males and females
#'
#' @param replist List object created by read_admb function
#' @return Plot of predicted mature male biomass
#' @export
plot_ssb <- function(replist){

  df <- get_ssb(replist)
  #A <- replist
#
  #dfpar   <- data.frame(par=A$fit$names,log_mmb=A$fit$est,log_sd=A$fit$std)
  #df      <- subset(dfpar,par=="sd_log_mmb")[,-1]
  #df$year <- A$mod_yrs
  #df$lb   <- exp(df$log_mmb - 1.96*df$log_sd)
  #df$ub   <- exp(df$log_mmb + 1.96*df$log_sd)
#
  p <- ggplot(df)
  p <- p + geom_line(aes(x=year,y=exp(log_mmb)))
  p <- p + geom_ribbon(aes(x=year,ymax=ub,ymin=lb),alpha=0.3)
  p <- p + labs(x="Year",y="Spawning biomass")

  pSSB <- p + ggtheme
  return(pSSB)
}
