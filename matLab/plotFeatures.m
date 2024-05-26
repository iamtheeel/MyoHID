function plotFeatures(features, startTrace, endTrace, legendString, titleStr, dataDir)
    fig = figure();
    %yyaxis left
    plot(features(:,startTrace))
    hold;
    for i = startTrace+1:endTrace
        plot(features(:,i));
    end
    yyaxis right
    plot(features(:,size(features,2))); % The labels
    ylabel('Label')

    legend(legendString, Location="southoutside", Orientation="horizontal");
    title(titleStr);
    hold off;

    if(dataDir ~= "")
        saveas(fig, strcat(dataDir, '/figures/features_', titleStr, '.png'))
    end
end