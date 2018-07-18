library(dplyr)
library(ggplot2)
#library(lubridate)
library(scales)

in_bbat <- read.csv("data/BBAT_009.CSV", skip = 1, stringsAsFactors = FALSE)
in_bbat$dh <- strftime(in_bbat$date.time, '%Y-%m-%d %H:00:00')
bbat <- in_bbat %>% group_by(dh) %>% summarize(bbat = mean(PM2.5atm.ug.m3, na.rm = TRUE),num_hour = sum(!is.na(PM2.5atm.ug.m3)))


in_pies <- read.csv("data/PIES_007.CSV", skip = 1, stringsAsFactors = FALSE)
in_pies$dh <- strftime(in_pies$date.time, '%Y-%m-%d %H:00:00')
pies <- in_pies %>% group_by(dh) %>% summarize(pies = mean(PM2.5atm.ug.m3, na.rm = TRUE))

in_robb <- read.csv("data/ROBB_007.CSV", skip = 1, stringsAsFactors = FALSE)
in_robb$dh <- strftime(in_robb$date.time, '%Y-%m-%d %H:00:00')
robb <- in_robb %>% group_by(dh) %>% summarize(robb = mean(PM2.5atm.ug.m3, na.rm = TRUE))

in_rpsu <- read.csv("data/RPSU_007.CSV", skip = 1, stringsAsFactors = FALSE)
in_rpsu$dh <- strftime(in_rpsu$date.time, '%Y-%m-%d %H:00:00')
rpsu <- in_rpsu %>% group_by(dh) %>% summarize(rpsu = mean(PM2.5atm.ug.m3, na.rm = TRUE))

in_tree <- read.csv("data/TREE_007.CSV", skip = 1, stringsAsFactors = FALSE)
in_tree$dh <- strftime(in_tree$date.time, '%Y-%m-%d %H:00:00')
tree <- in_tree %>% group_by(dh) %>% summarize(tree = mean(PM2.5atm.ug.m3, na.rm = TRUE))


sel <- read.csv("data/deqsel_pm25_hrly_20180531_to_20180606.csv", skip = 3, stringsAsFactors = FALSE)
sel$dh <- strptime(sel$X, ' %m/%d/%Y %H:%M')

colnames(sel) <- c("datetime", "pm25", "dh")
df <- merge(sel, bbat, by = "dh", all.x = TRUE)
df <- merge(df, pies, by = "dh", all.x = TRUE)
df <- merge(df, robb, by = "dh", all.x = TRUE)
df <- merge(df, rpsu, by = "dh", all.x = TRUE)
df <- merge(df, tree, by = "dh", all.x = TRUE)

rbbat <- lm(data = df,bbat ~ pm25)
summary(rbbat)
rpies <- lm(data = df, pies ~ pm25)
summary(rpies)
rrobb <- lm(data = df, robb ~ pm25)
summary(rrobb)
rrpsu <- lm(data = df, rpsu ~ pm25)
summary(rrpsu)
rtree<- lm(data = df, tree ~ pm25)
summary(rtree)

write.csv(df, "calib5_sel.csv")

#############################################################################
##
## COMPARING 5-MINUTE DATA
##
#############################################################################

##read in the DEQ SEL 5-minute data
in_sel5 <- read.csv("data/deqsel_pm25_5min.csv", skip = 1, stringsAsFactors = FALSE)
in_sel5$dt <- strptime(in_sel5$Date.Time, ' %m/%d/%Y %H:%M')
colnames(in_sel5) <- c("mdy", "pm25", "dt")
in_sel5$pm25 <- as.numeric(in_sel5$pm25)

## create the 5-minute averages for the monitors
create_5min_avg <- function(df, mon) {
  df$min5 <- trunc(as.numeric(strftime(df$date.time, '%M'))/5)
  df_min5 <- df %>% group_by(dh, min5) %>% 
    summarize(pmmed = median(PM2.5atm.ug.m3, na.rm = TRUE),
              pmavg = mean(PM2.5atm.ug.m3, na.rm = TRUE),
              num_min5 = sum(!is.na(PM2.5atm.ug.m3))) %>%
    ungroup()
  colnames(df_min5)[which(colnames(df_min5) == 'pmavg')] <- paste0(mon, "_avg")
  colnames(df_min5)[which(colnames(df_min5) == 'pmmed')] <- paste0(mon, "_med")
  df_min5$dtstr <- paste0(strftime(df_min5$dh, '%Y-%m-%d %H'), ":", as.character(df_min5$min5*5))
  df_min5$dt <- strptime(df_min5$dtstr, '%Y-%m-%d %H:%M')
# DEQ SEL time is in PST, so convert PDT to PST
#  df_min5$dt$hour <- df_min5$dt$hour - 1
  df_min5$dt$min <- df_min5$dt$min - 55
  return(df_min5 )
  
}

bbat_min5 <- create_5min_avg(in_bbat, "bbat")
pies_min5 <- create_5min_avg(in_pies, "pies")
robb_min5 <- create_5min_avg(in_robb, "robb")
rpsu_min5 <- create_5min_avg(in_rpsu, "rpsu")
tree_min5 <- create_5min_avg(in_tree, "tree")

## merge the sensor & DEQ SEL 5-minute data, and do a linear regression
df5 <- merge(in_sel5, bbat_min5[, c("dt", "bbat_avg", "bbat_med")], by = "dt", all = TRUE)
dim(df5)
df5 <- merge(df5,pies_min5[, c("dt", "pies_avg", "pies_med")], by = "dt", all = TRUE)
dim(df5)
df5 <- merge(df5,robb_min5[, c("dt", "robb_avg", "robb_med")], by = "dt", all = TRUE)
dim(df5)
df5 <- merge(df5,rpsu_min5[, c("dt", "rpsu_avg", "rpsu_med")], by = "dt", all = TRUE)
dim(df5)
df5 <- merge(df5,tree_min5[, c("dt", "tree_avg", "tree_med")], by = "dt", all = TRUE)
dim(df5)
write.csv(df5, "data/calib_min5.csv")

summary(lm(data = df5, bbat_avg ~ pm25))
summary(lm(data = df5, pies_avg ~ pm25))
summary(lm(data = df5, robb_avg ~ pm25))
summary(lm(data = df5, rpsu_avg ~ pm25))
summary(lm(data = df5, tree_avg ~ pm25))

sens <- list("bbat_avg", "pies_avg", "robb_avg", "rpsu_avg", "tree_avg")
for ( sen in sens) {
  tstr <- "Co-location run 2018-05-31 to 2018-06-06 at DEQ SEL : 5 minute data"
  xstr <- paste0("Sensor ", substring(sen, 1, 4), " PM2.5 (ug/m3)")
  sen_pm25 <- df5[, which(colnames(df5) == sen)]
  res <- lm(data = df5, pm25 ~ sen_pm25 )
  r <- summary(res)
  lb1 <- paste0("SEL = ", round(res$coefficients["sen_pm25"], 3), " * ", substring(sen, 1, 4), " + ", 
                round(res$coefficients["(Intercept)"], 3), "\n")
  lb2 <- paste("R^2 == ", round(r$adj.r.squared, 3)) 
  g <- ggplot(df5, aes(sen_pm25, pm25)) + geom_point(size = 1, col = "#999999") +
    geom_smooth(method = "lm", col = "#545454", lwd = 0.7) +
    geom_smooth( aes(x = sen_pm25, y = pm25, col = "#ff4040"), method = "lm", lwd = 0.7) +
    labs(title=tstr,x=xstr, y = "DEQ SEL PM2.5 (ug/m3)") + 
    theme(plot.title = element_text(size = 18)) +
    theme(legend.position="none") + 
    theme(axis.text=element_text(size=12),
          axis.title=element_text(size=14,face="bold")) +
    annotate("text", label = lb1, x = 5.8, y = 20, col = "#666666", size = 5) +
    annotate("text", label = lb2, x = 5.8, y = 19.2, col = "#666666", size = 5, parse=TRUE)
  png(file=paste0("data/",substring(sen, 1, 4), ".png"), width=700)
  plot(g)
  dev.off()
}




