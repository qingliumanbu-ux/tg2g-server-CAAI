/*=========================================================================
//程序名称:     f_caai_getcb
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：根据钢种、标准、全程途径码计算定额成本
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
#include "tcaais2.h" 
//程序用头文件

BM2_FUNCTION_EXPORT
int f_caai_getcb(CString account_period, CString price_terms,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString   sub_backlog_code = "";
	CString   whole_backlog_desc = "";
	CString   whole_backlog = "";
	CString   mat_name = "";
	CString v_error = "";
	
	

	CModel tcaac05b("TCAAC05B");

	// 创建电文处理对象
	CDbCommand cmd_inq_b(conn);
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	try
	{
		tcaac05b["ACCOUNT_PERIOD"] = account_period;
		tcaac05b["PRICE_TERMS"] = price_terms;
		tcaac05b.Delete("ACCOUNT_PERIOD,PRICE_TERMS");

		//效率问题，直接使用插入的方式,如果就只是炼钢工序，则产品规格范围及坯料规格基本都是一致的
		sqlstr = " insert into tcaac05b(ACCOUNT_PERIOD,REC_CREATE_TIME,REC_CREATOR,PRICE_TERMS"
			",MSC,MSC_LINE_NO,prod_code,prod_cname,sg_std,sg_sign,std_sg_code"
			",MIN_THICK,MAX_THICK,MIN_WIDTH,MAX_WIDTH,MIN_LEN,MAX_LEN,whole_backlog,MAT_CODE"
			",psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE)"
			" select distinct @account_period,@rec_create_time,@rec_creator,@price_terms"
			",MSC,MSC_LINE_NO,prod_code,prod_cname,sg_std,sg_sign,std_sg_code"
			",MIN_THICK,MAX_THICK,MIN_WIDTH,MAX_WIDTH,MIN_LEN,MAX_LEN,whole_backlog,MAT_CODE"
			",psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE"
			" from ("
			"select @account_period,@rec_create_time,@rec_creator,@price_terms"
			",MSC,MSC_LINE_NO,prod_code,prod_cname,sg_std,sg_sign,std_sg_code"
			",MIN_THICK,MAX_THICK,MIN_WIDTH,MAX_WIDTH,MIN_LEN,MAX_LEN,whole_backlog,SLAB_THICK,SLAB_WIDTH,nvl(INGOT_CODE,' '),case when SLAB_THICK=SLAB_WIDTH AND nvl(INGOT_CODE,' ')!=' ' then 'A6' ELSE 'A7' END MAT_CODE"
			",nvl(psc,' ') psc,nvl(DELIVY_STATUS_CODE,'0') DELIVY_STATUS_CODE,nvl(HOT_TREAT_METHOD_CODE,'0') HOT_TREAT_METHOD_CODE"
			" from ("
			//取产品代码对应得工艺路径及冶金规范和消耗量
			 " select t04.MSC,t04.MSC_LINE_NO,t04.IDX_NO as idx_no_a2"  //冶金规范、产线号
			",t01.prod_code, t01.prod_cname,t01.sg_std,t01.sg_sign,t01.std_sg_code" //产品大类信息
			",ta2.MIN_THICK,ta2.MAX_THICK,ta2.MIN_WIDTH,ta2.MAX_WIDTH,ta2.MIN_LEN,ta2.MAX_LEN"  //产品规格范围
			",t02.whole_backlog" // --全程工序代码
			",pl.INGOT_CODE,pl.SLAB_THICK,pl.SLAB_WIDTH"  // --坯料的规格
			",tp03.psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE" //--产品规范码
			" from TQMTm04 t04"  //冶金规范基表索引表(产品规格)
			" left join tqmtm01 t01 on t01.msc = t04.msc" //产品大类信息
			" left join TQMTm02 t02 on t02.msc = t04.msc and t02.MSC_LINE_NO = t04.MSC_LINE_NO" //工艺路径
			" left join TQMTmA2 ta2 on t04.IDX_NO = ta2.IDX_NO"  //--产线适用规格表
			" left join "
			" ("
			" SELECT MSC, MSC_LINE_NO, IDX_NO as idx_no_pl FROM TQMTm04 WHERE BASIC_TABLE_CODE = 'AY' and whole_Backlog_code != 'B0'"
			" ) t2  on t04.msc = t2.msc and t04.MSC_LINE_NO = t2.MSC_LINE_NO "   //坯料索引号
			" left join TQMTMAY pl on t2.idx_no_pl = pl.IDX_NO"  //坯料规格
			" left join ( select tp03.msc,tp03.psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE from tqmtp03  tp03 left join TQMTP01 tp01 on tp03.psc=tp01.psc) tp03 on t04.msc = tp03.msc"
			" where 1=1"
			" and t04.idx_no != ' '"
			" and t04.BASIC_TABLE_CODE = 'A2'"
			" order by t04.MSC, t04.MSC_LINE_NO, t02.WHOLE_BACKLOG"
			" )"
			" where nvl(INGOT_CODE,' ')!=' '"
			" and nvl(prod_code, ' ') != ' '"
			" and nvl(MIN_THICK, 99999999) != 99999999"
			" and whole_backlog !='A1'"
			")"
			;
		Log::Trace("", __FUNCTION__, "sqlstr = [{0}] ",sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//效率问题，直接使用插入的方式,如果就只是炼钢工序，则产品规格范围及坯料规格基本都是一致的
		sqlstr = " insert into tcaac05b(ACCOUNT_PERIOD,REC_CREATE_TIME,REC_CREATOR,PRICE_TERMS"
			",MSC,MSC_LINE_NO,prod_code,prod_cname,sg_std,sg_sign,std_sg_code"
			",MIN_THICK,MAX_THICK,MIN_WIDTH,MAX_WIDTH,MIN_LEN,MAX_LEN,whole_backlog,MAT_CODE"
			",psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE)"
			" select distinct @account_period,@rec_create_time,@rec_creator,@price_terms"
			",MSC,MSC_LINE_NO,prod_code,prod_cname,sg_std,sg_sign,std_sg_code"
			",MIN_THICK,MAX_THICK,MIN_WIDTH,MAX_WIDTH,MIN_LEN,MAX_LEN,whole_backlog,MAT_CODE"
			",psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE"
			" from ("
			"select @account_period,@rec_create_time,@rec_creator,@price_terms"
			",MSC,MSC_LINE_NO,prod_code,prod_cname,sg_std,sg_sign,std_sg_code"
			",MIN_THICK,MAX_THICK,MIN_WIDTH,MAX_WIDTH,MIN_LEN,MAX_LEN,whole_backlog,SLAB_THICK,SLAB_WIDTH,nvl(INGOT_CODE,' '),case when SLAB_THICK=SLAB_WIDTH AND nvl(INGOT_CODE,' ')!=' ' then 'A6' ELSE 'A7' END MAT_CODE"
			",nvl(psc,' ') psc,nvl(DELIVY_STATUS_CODE,'0') DELIVY_STATUS_CODE,nvl(HOT_TREAT_METHOD_CODE,'0') HOT_TREAT_METHOD_CODE"
			" from ("
			//取产品代码对应得工艺路径及冶金规范和消耗量
			" select t04.MSC,t04.MSC_LINE_NO,t04.IDX_NO as idx_no_a2"  //冶金规范、产线号
			",t01.prod_code, t01.prod_cname,t01.sg_std,t01.sg_sign,t01.std_sg_code" //产品大类信息
			",ta2.MIN_THICK,ta2.MAX_THICK,ta2.MIN_WIDTH,ta2.MAX_WIDTH,ta2.MIN_LEN,ta2.MAX_LEN"  //产品规格范围
			",t02.whole_backlog" // --全程工序代码
			",pl.INGOT_CODE,pl.SLAB_THICK,pl.SLAB_WIDTH"  // --坯料的规格
			",tp03.psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE" //--产品规范码
			" from TQMTm04 t04"  //冶金规范基表索引表(产品规格)
			" left join tqmtm01 t01 on t01.msc = t04.msc" //产品大类信息
			" left join TQMTm02 t02 on t02.msc = t04.msc and t02.MSC_LINE_NO = t04.MSC_LINE_NO" //工艺路径
			" left join TQMTmA2 ta2 on t04.IDX_NO = ta2.IDX_NO"  //--产线适用规格表
			" left join "
			" ("
			" SELECT MSC, MSC_LINE_NO, IDX_NO as idx_no_pl FROM TQMTm04 WHERE BASIC_TABLE_CODE = 'AY' and whole_Backlog_code = 'B0'"
			" ) t2  on t04.msc = t2.msc and t04.MSC_LINE_NO = t2.MSC_LINE_NO "   //坯料索引号
			" left join TQMTMAY pl on t2.idx_no_pl = pl.IDX_NO"  //坯料规格
			" left join ( select tp03.msc,tp03.psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE from tqmtp03  tp03 left join TQMTP01 tp01 on tp03.psc=tp01.psc) tp03 on t04.msc = tp03.msc"
			" where 1=1"
			" and t04.idx_no != ' '"
			" and t04.BASIC_TABLE_CODE = 'A2'"
			" order by t04.MSC, t04.MSC_LINE_NO, t02.WHOLE_BACKLOG"
			" )"
			" where nvl(INGOT_CODE,' ')!=' '"
			" and nvl(prod_code, ' ') != ' '"
			" and nvl(MIN_THICK, 99999999) != 99999999"
			" and whole_backlog ='A1B0'"
			")"
			;
		Log::Trace("", __FUNCTION__, "sqlstr = [{0}] ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//仅有炼钢产线的
		sqlstr = " insert into tcaac05b(ACCOUNT_PERIOD,REC_CREATE_TIME,REC_CREATOR,PRICE_TERMS"
			",MSC,MSC_LINE_NO,prod_code,prod_cname,sg_std,sg_sign,std_sg_code"
			",MIN_THICK,MAX_THICK,MIN_WIDTH,MAX_WIDTH,MIN_LEN,MAX_LEN,whole_backlog,INGOT_CODE,SLAB_THICK,SLAB_WIDTH"
			",psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE,MAT_CODE)"
			"select distinct @account_period,@rec_create_time,@rec_creator,@price_terms"
			",MSC,MSC_LINE_NO,prod_code,prod_cname,sg_std,sg_sign,std_sg_code"
			",SLAB_THICK,SLAB_THICK,SLAB_WIDTH,SLAB_WIDTH,0,999999,whole_backlog,INGOT_CODE,SLAB_THICK,SLAB_WIDTH"
			",nvl(psc,' ') psc,nvl(DELIVY_STATUS_CODE,'0') DELIVY_STATUS_CODE,nvl(HOT_TREAT_METHOD_CODE,'0') HOT_TREAT_METHOD_CODE,DECODE(SLAB_THICK,SLAB_WIDTH,'A6','A7')"
			" from ("
			//取产品代码对应得工艺路径及冶金规范和消耗量
			" select t04.MSC,t04.MSC_LINE_NO,t04.IDX_NO as idx_no_a2"  //冶金规范、产线号
			",t01.prod_code, t01.prod_cname,t01.sg_std,t01.sg_sign,t01.std_sg_code" //产品大类信息
			",t02.whole_backlog" // --全程工序代码
			",pl.INGOT_CODE,pl.SLAB_THICK,pl.SLAB_WIDTH"  // --坯料的规格
			",tp03.psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE" //--产品规范码
			" from tqmtmd9 pl,TQMTm04 t04"  //冶金规范基表索引表(产品规格)
			" left join tqmtm01 t01 on t01.msc = t04.msc" //产品大类信息
			" left join TQMTm02 t02 on t02.msc = t04.msc and t02.MSC_LINE_NO = t04.MSC_LINE_NO" //工艺路径			
			" left join ( select tp03.msc,tp03.psc,DELIVY_STATUS_CODE,HOT_TREAT_METHOD_CODE from tqmtp03  tp03 left join TQMTP01 tp01 on tp03.psc=tp01.psc) tp03 on t04.msc = tp03.msc"
			" where 1=1"
			" and t02.whole_backlog ='A1'"
			" and pl.billet_type = '3'"
			" and t04.idx_no != ' '"
			" and t04.BASIC_TABLE_CODE = 'A2'"
			" order by t04.MSC, t04.MSC_LINE_NO, t02.WHOLE_BACKLOG"
			" )"
			" where nvl(INGOT_CODE,' ')!=' '"
			" and nvl(prod_code, ' ') != ' '"
			;
		Log::Trace("", __FUNCTION__, "sqlstr = [{0}] ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " update  tcaac05b t1 set REFINE_ROUTE_CODE = (SELECT distinct REFINE_ROUTE_CODE  FROM"
			" (select tm04.msc, tm04.MSC_LINE_NO, tmac.st_no, ts0x.REFINE_ROUTE_CODE"
			" from TQMTM04 tm04"
			" left join TQMTMAC tmac on tm04.IDX_NO = tmac.IDX_NO"
			" left join TQMTS0X ts0x on tmac.ST_NO = ts0x.ST_NO ) t2 where t1.msc =t2.msc and t1.MSC_LINE_NO = t2.MSC_LINE_NO and rownum=1)"
			" where 1=1"
			" and exists(select 1 from (select tm04.msc, tm04.MSC_LINE_NO, tmac.st_no, ts0x.REFINE_ROUTE_CODE"
			" from TQMTM04 tm04"
			" left join TQMTMAC tmac on tm04.IDX_NO = tmac.IDX_NO"
			" left join TQMTS0X ts0x on tmac.ST_NO = ts0x.ST_NO where nvl(ts0x.REFINE_ROUTE_CODE,' ')!=' ') t2 where t1.msc =t2.msc and t1.MSC_LINE_NO = t2.MSC_LINE_NO)"
			
			" and  PRICE_TERMS = @price_terms"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//更新牌号索引和标准索引
		sqlstr = " update tcaac05b t1 set sg_seq = (SELECT SG_SEQ  FROM TQMTPA4 t2  WHERE t1.SG_SIGN=t2.SG_SIGN)"
			" where 1=1"
			" and exists(SELECT 1  FROM TQMTPA4 t2  WHERE t1.SG_SIGN=t2.SG_SIGN)"
			" and  PRICE_TERMS = @price_terms"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		Log::Trace("", __FUNCTION__, "22222222 ");

		sqlstr = " update tcaac05b t1 set std_code = (SELECT std_code  FROM TQMTPA5 t2  WHERE t1.SG_STD=t2.SG_STD)"
			" where 1=1"
			" and exists(SELECT 1  FROM TQMTPA5 t2  WHERE t1.SG_STD=t2.SG_STD)"
			" and  PRICE_TERMS = @price_terms"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();	

		Log::Trace("", __FUNCTION__, "33333333333333 ");

		//更新坯料物料编码
		sqlstr = " update tcaac05b t1 set MAT_CODE = MAT_CODE||std_code||sg_seq"
			",product_code = prod_code||std_code||sg_seq||DELIVY_STATUS_CODE||HOT_TREAT_METHOD_CODE"
			" where 1=1"
			" and sg_seq!=' '"
			" and std_code!=' '"
			" and  PRICE_TERMS = @price_terms"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		Log::Trace("", __FUNCTION__, "444444444444444 ");

		//判断单价是否都存在
		sqlstr = " select distinct t1.mat_code,t2.mat_name"
			" from tcaac05c t1"
			" left join tcaac11 t2 on t1.mat_code = t2.mat_code "
			" where 1=1"
			" and t1.mat_code not in (select mat_code from tcaac12 where PRICE_TERMS = @price_terms AND VALID_TIME_START <= @account_period||'31' )"
			" and t1.product_code in (select mat_code from tcaac05b where account_period = @account_period)"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteReader();
		v_error = "";
		while (cmd_inq.Read())
		{
			v_error = v_error + "物料编码[" + cmd_inq.GetString(1) + "]名称[" + cmd_inq.GetString(2) + "]";			
		}
		cmd_inq.Close();

		
		if (v_error != "")
		{
			v_error = v_error + "在该价格类型下的单价未维护!";
			sprintf(s.msg, v_error);
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		Log::Trace("", __FUNCTION__, "55555555555555 ");

		//更新合金消耗

		sqlstr = " update tcaac05b t0 set ALLOY_COST "
			" = (select round(sum(nvl(t2.price_unit,0)*USE_UNIT),2) AS  AMT from tcaac05c t1"
			  " left join"
			  " (SELECT mat_code, price_unit from  tcaac12 where(mat_code, VALID_TIME_START) in"
			  " (select mat_code, max(VALID_TIME_START) from tcaac12"
			  " where VALID_TIME_START <= @account_period||'31' and PRICE_TERMS = @price_terms group by mat_code)"
			  " and  PRICE_TERMS = @price_terms"
			" ) t2 on t1.mat_code = t2.mat_code "
			" where t1.mat_code LIKE 'Y03%'  and t1.product_code = t0.mat_code"
			")"
			" where mat_code !=' '"
			" and exists(select 1 from tcaac05c t1 where t1.mat_code LIKE 'Y03%'  and  t1.product_code= t0.mat_code)"
			" and price_terms =@price_terms"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " update tcaac05b t0 set GTL_COST "
			" = (select round(sum(nvl(t2.price_unit,0)*USE_UNIT/1000),2) AS  AMT from tcaac05c t1"
			" left join"
			" (SELECT mat_code, price_unit from  tcaac12 where(mat_code, VALID_TIME_START) in"
			" (select mat_code, max(VALID_TIME_START) from tcaac12"
			" where VALID_TIME_START <= @account_period||'31' and PRICE_TERMS = @price_terms group by mat_code)"
			" and  PRICE_TERMS = @price_terms"
			" ) t2 on t1.mat_code = t2.mat_code "
			" where t1.mat_code NOT LIKE 'Y03%' and t1.product_code = t0.mat_code"
			")"
			" where mat_code!=' ' "
			" and exists(select 1 from tcaac05c t1 where t1.mat_code not LIKE 'Y03%'  and t1.product_code= t0.mat_code)"
			" and price_terms =@price_terms"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		Log::Trace("", __FUNCTION__, "66666666 ");

		//循环太慢，RH精炼加工费
		sqlstr = " select BACK_N1 AS AMT from tcaaib1"
			" where PROJECT_ID = 'JG'"
			" and BACK_C1 <= @account_period"
			" AND BACK_C2  = 'R'"
			" order by BACK_C1 desc"
			;
		//Log::Trace("", __FUNCTION__, "sub_backlog_code = [{0}],sqlstr=[{1}]  ", sub_backlog_code, sqlstr);
		cmd_inq_1.SetCommandText(sqlstr);
		cmd_inq_1.Parameters.Set("account_period", account_period);
		cmd_inq_1.ExecuteReader();
		if (cmd_inq_1.Read())
		{
			sqlstr = " update tcaac05b set RH_COST = decode(REFINE_ROUTE_CODE,'RLR',2,1) * @fy_use"
				" where REFINE_ROUTE_CODE in ('R','LR','RL','RLR')"
				" and price_terms =@price_terms"
				" and account_period = @account_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("account_period", account_period);
			cmd_inq.Parameters.Set("price_terms", price_terms);
			cmd_inq.Parameters.Set("fy_use", cmd_inq_1.GetDecimal(1));
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}
		cmd_inq_1.Close();

		//更新各工序加工费
		sqlstr = " select distinct WHOLE_BACKLOG from tcaac05b"
			" where 1=1"
			" and price_terms =@price_terms"
			" and account_period = @account_period"
			;
		;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tcaac05b.Reset();
			cmd_inq.Fetch(tcaac05b);
			tcaac05b["ACCOUNT_PERIOD"] = account_period;
			tcaac05b["PRICE_TERMS"] = price_terms;

			//成材率的查找
			whole_backlog = tcaac05b["WHOLE_BACKLOG"].ToString();
			whole_backlog_desc = "";
			//Log::Trace("", __FUNCTION__, "whole_backlog = [{0}],whole_backlog.GetLength()=[{1}]  ", whole_backlog, whole_backlog.GetLength());
			for (int j = 0; j < whole_backlog.GetLength(); j = j + 2) //全程工序代码截取
			{
				//Log::Trace("", __FUNCTION__, "whole_backlog = [{0}],j=[{1}]  ", whole_backlog,j);
				sub_backlog_code = whole_backlog.SubstringNE(j, 2);

				if (whole_backlog_desc.Trim() == "")
				{
					whole_backlog_desc = sub_backlog_code;
				}
				else
				{
					whole_backlog_desc = whole_backlog_desc + "," + sub_backlog_code;
				}

				if (sub_backlog_code == "B0" || sub_backlog_code == "B1" || sub_backlog_code == "B2" || sub_backlog_code == "B3" || sub_backlog_code == "D1")
				{
					//取成材率
					sqlstr = " select BACK_N1 AS AMT from tcaaib1"
						" where PROJECT_ID = 'CC'"
						" and BACK_C1 = (SELECT MAX(BACK_C1) FROM TCAAIB1 WHERE BACK_C1<=@account_period)"
						" AND BACK_C2  = @sub_backlog_code"
						" order by BACK_C1 desc"
						;
					//Log::Trace("", __FUNCTION__, "sub_backlog_code = [{0}],sqlstr=[{1}]  ", sub_backlog_code, sqlstr);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("account_period", account_period);
					cmd_inq_1.Parameters.Set("price_terms", price_terms);
					cmd_inq_1.Parameters.Set("sub_backlog_code", sub_backlog_code);
					cmd_inq_1.ExecuteReader();
					if (cmd_inq_1.Read())
					{
						if (tcaac05b["CCL"].ToDecimal() == 0)
						{
							tcaac05b["CCL"] = cmd_inq_1.GetDecimal(1);
						}
						else
						{
							tcaac05b["CCL"] = cmd_inq_1.GetDecimal(1);
							//tcaac05b["CCL"] = tcaac05b["CCL"].ToDecimal()*cmd_inq_1.GetDecimal(1)/100;
						}
					}
					cmd_inq_1.Close();
				}

				//各工序的加工费
				sqlstr = " select BACK_N1 AS AMT from tcaaib1"
					" where 1=1"
					" AND BACK_C1  <= @account_period"
					" AND BACK_C2  = @sub_backlog_code"
					" AND PROJECT_ID = 'JG'"
					" ORDER BY BACK_C1 DESC"
					;
				//Log::Trace("", __FUNCTION__, "sub_backlog_code = [{0}]  ", sub_backlog_code);
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("sub_backlog_code", sub_backlog_code);
				cmd_inq_1.ExecuteReader();
				if (cmd_inq_1.Read())
				{
					mat_name = sub_backlog_code + "_COST";
					tcaac05b[mat_name] = cmd_inq_1.GetDecimal(1).Round(2);
				}
				cmd_inq_1.Close();
			}

			//期间成本		
			sqlstr = " select BACK_N1 AS AMT from tcaaib1"
				" where 1=1"
				" AND  BACK_C2  = 'QJ'"
				" AND BACK_C1  <= @account_period"
				" AND PROJECT_ID = 'JG'"
				" ORDER BY BACK_C1 DESC"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.ExecuteReader();
			if (cmd_inq_1.Read())
			{
				tcaac05b["QJ_COST"] = cmd_inq_1.GetDecimal(1).Round(2);
			}
			cmd_inq_1.Close();
			
			tcaac05b["CCL"] = tcaac05b["CCL"].ToDecimal().Round(3);

			tcaac05b["WHOLE_BACKLOG_DESC"] = whole_backlog_desc;
			tcaac05b.Update("WHOLE_BACKLOG_DESC,A1_COST,B0_COST, B1_COST, B2_COST, B3_COST, D1_COST, CCL, H1_COST, H2_COST, H3_COST, H4_COST, L1_COST, S0_COST, T1_COST, T2_COST, T3_COST, T4_COST, T5_COST, T6_COST, T7_COST, Y1_COST, QJ_COST, S1_COST ", "ACCOUNT_PERIOD,PRICE_TERMS,WHOLE_BACKLOG");

		}
		cmd_inq.Close();

		//更新总金额
		sqlstr = "update tcaac05b set PRICE_UNIT = round((A1_COST+ALLOY_COST+GTL_COST+RH_COST)/decode(CCL,0,100,CCL)*100,2)"
			"+B0_COST+B1_COST+B2_COST+B3_COST+D1_COST+H1_COST+H2_COST+H3_COST+H4_COST+L1_COST"
			"+S0_COST+T1_COST+T2_COST+T3_COST+T4_COST+T5_COST+T6_COST+T7_COST+Y1_COST+QJ_COST"
			" where 1=1"
			" and  PRICE_TERMS = @price_terms"
			" and account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
