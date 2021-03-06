#!/usr/bin/python                                                                                     

import pandas as pd
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
plt.ioff()
import matplotlib.cm as cm
import cartopy
import cartopy.io.shapereader as shpreader
import cartopy.crs as ccrs
import os
import sys
from tqdm import tqdm

DATAFILE="INFECT_stats.csv"
SHPFILE='UScounties/UScounties.shp'
OUTFILE='res.pdf'
CMAP='jet'
LOG=True


total = len(sys.argv)
if total > 1:
    DATAFILE = sys.argv[1]
if total > 2:
    CMAP = sys.argv[2]
if total > 3:
    OUTFILE = sys.argv[3]

AUC='AUC'


cmap = plt.get_cmap(CMAP)
vmin=0.5
vmax=0.65
if LOG:
    norm=mpl.colors.LogNorm(vmin=vmin,vmax=vmax)
else:
    norm=mpl.colors.Normalize(vmin=vmin,vmax=vmax)
cmapS = cm.ScalarMappable(norm=norm, cmap=cmap)

df=pd.read_csv(DATAFILE)
df.FIPS=[str(int(i)).zfill(5) for i in df.FIPS]
fipsdict={fips:auc for fips,auc in zip(df.FIPS.values,df[AUC].values)}

fig=plt.figure(figsize=(20,15))
ax = plt.axes([0, 0, 1, 1],
                projection=ccrs.LambertConformal())

ax.set_extent([-119, -74.5, 22.5, 50], ccrs.Geodetic())
ax.background_patch.set_visible(False)
ax.outline_patch.set_visible(False)
shpf = shpreader.Reader(SHPFILE)
    
for record, state in tqdm(zip(shpf.records(), shpf.geometries())):
    try:
        colorNormalized = cmapS.to_rgba(fipsdict[record.attributes['FIPS']])

        ax.add_geometries([state], ccrs.PlateCarree(), 
                  facecolor=colorNormalized, edgecolor=None,linewidth=0,)
    except KeyError:
        ax.add_geometries([state], ccrs.PlateCarree(),
          facecolor='.7', edgecolor=None,linewidth=0)    
    
sm = plt.cm.ScalarMappable(norm=norm,cmap=cmap)
sm._A = []
cb = plt.colorbar(sm,ticks=[vmin,vmin+(vmax-vmin)*.25,
                            vmin+(vmax-vmin)*.5,vmin+(vmax-vmin)*.75,
                            vmax],shrink=0.35, pad = 0.0,
                            cax=fig.add_axes([0.93,0.3,0.01,.3]))
cb.ax.set_yticklabels(cb.ax.get_yticklabels(), fontsize=20);

ax.set_axis_off()
fig.subplots_adjust(left=0, right=1, bottom=0, top=1)
plt.margins(0,0)
ax.xaxis.set_major_locator(mpl.ticker.NullLocator())
ax.yaxis.set_major_locator(mpl.ticker.NullLocator())
plt.savefig(OUTFILE,dpi=600, bbox_inches='tight',transparent=True,pad_inches = 0)
