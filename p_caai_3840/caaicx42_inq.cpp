/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2018-06-21
Description:查询轧钢按工序查询信息
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx42_inq)
//-EP_SYSTEM_HEAD_END

int f_caaicx42_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString		begin_time = " ";
	CString		end_time = " ";
	CString		mat_code = " ";
	CString		price_trems = "YJ";
	CString		dept_code = " ";
	CString		flag = "0";
	CString		flag2 = "0";
	int rownum = 1;
	CDecimal wt2 = 0;
	CModel tcaai04("TCAAI04");


	CString  sqlstr("");
	CString  sqlstr_where("");
	CString  sqlstr_sub("");
	CString  sqlstr_fl("");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();
		price_trems = bcls_rec->Tables[0].Rows[0]["PRICE_TERMS"].ToString();
		flag = bcls_rec->Tables[0].Rows[0]["FLAG"].ToString(); //用来判断是单耗还是消耗	
		flag2 = bcls_rec->Tables[0].Rows[0]["FLAG2"].ToString(); //用来判断是否含异动	
		tcaai04.MergeFrom(bcls_rec->Tables[0].Rows[0]);
	

		//1、各明细消耗项的消耗字段查询,扣除钢坯的信息（暂定钢坯的物料代码字段长度超过20字符的）
		//如SUM(case when mat_code= 'NY001' then WT else 0 end)  as 'NY001_USE'
		sqlstr = " select  distinct mat_code"
			" from tcaai04 "
			" WHERE	1=1 "
			" and mat_code !=' '"
			" and length(mat_code)<20"
			;
		sqlstr = sqlstr + sqlstr_where;
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("sg_sign", tcaai04["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaai04["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai04["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("vfree1", tcaai04["VFREE1"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai04["DEPT_CODE"].ToString());
		cmd_inq.ExecuteReader();
		if (flag == "1")
		{
			sqlstr_sub = ",SUM(case when t3.mat_code= 'GP_CODE' and all_outwt !=0 then round(use_wt/all_outwt,6) else 0 end)  as GP_USE";
		}
		else
		{
			sqlstr_sub = ",SUM(case when t3.mat_code= 'GP_CODE' and all_outwt !=0 then round(use_wt*outwt/all_outwt,6) else 0 end)  as GP_USE";
		}
		
		while (cmd_inq.Read())
		{
			mat_code = cmd_inq.GetString(1);
			sqlstr = " select mat_type FROM TCAAC11"
				" WHERE mat_type in ('B','C','E')"
				" and MAT_CODE = @mat_code"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("mat_code", mat_code);
			cmd_inq_1.ExecuteReader();
			if (cmd_inq_1.Read())
			{
				if (flag == "1")
				{
					sqlstr_sub += ",SUM(case when t3.mat_code= '" + mat_code + "' and all_outwt !=0 then round(COST_YJ/all_outwt,2) else 0 end)  as " + mat_code + "_USE";
				}
				else
				{
					sqlstr_sub += ",SUM(case when t3.mat_code= '" + mat_code + "' and all_outwt !=0  then round(COST_YJ*outwt/all_outwt,2) else 0 end)  as " + mat_code + "_USE";
				}
			}
			else
			{
				if (flag == "1")
				{
					sqlstr_sub += ",SUM(case when t3.mat_code= '" + mat_code + "'  and all_outwt!=0  then round(use_wt/all_outwt,6) else 0 end)  as " + mat_code + "_USE";
				}
				else
				{
					sqlstr_sub += ",SUM(case when t3.mat_code= '" + mat_code + "' then round(outwt*use_wt/all_outwt,6) else 0 end)  as " + mat_code + "_USE";
				}
			}

			rownum++;
			if (rownum == 100)
			{
				break;
			}
		}
		cmd_inq.Close();


		if (flag == "1")
		{
			sqlstr_fl = " ,decode(all_outwt,0,0,round(cost_pl/all_outwt,2)) as cost_pl"
				" ,decode(all_outwt,0,0,round(cost_a/all_outwt,2)) as cost_a"
				" ,decode(all_outwt,0,0,round(cost_b/all_outwt,2)) as cost_b"
				" ,decode(all_outwt,0,0,round(cost_c/all_outwt,2)) as cost_c"
				" ,decode(all_outwt,0,0,round(cost_d/all_outwt,2)) as cost_d"
				" ,decode(all_outwt,0,0,round(cost_e/all_outwt,2)) as cost_e"
				;
		}
		else
		{
			sqlstr_fl = " ,decode(all_outwt,0,0,round(cost_pl*outwt/all_outwt,22)) as cost_pl"
				" ,decode(all_outwt,0,0,round(cost_a*outwt/all_outwt,2)) as cost_a"
				" ,decode(all_outwt,0,0,round(cost_b*outwt/all_outwt,2)) as cost_b"
				" ,decode(all_outwt,0,0,round(cost_c*outwt/all_outwt,2)) as cost_c"
				" ,decode(all_outwt,0,0,round(cost_d*outwt/all_outwt,2)) as cost_d"
				" ,decode(all_outwt,0,0,round(cost_e*outwt/all_outwt,2)) as cost_e"
				;
		}

		sqlstr = "select t1.stats_period, t1.relation_no AS VFREE1, t1.sub_backlog_code, t1.equ_no"
			", t1.STATS_PERIOD as prod_date"
			", t2.SG_SIGN,t2.MAT_THICK,t2.outwt AS WT,t2.prod_shift_no,t2.prod_shift_group" //产量
			" ,t5.all_outwt"
			",ts.ORD_CUST_CODE,ts.ORD_CUST_NAME"
			" ,decode(sum(case when t3.mat_code='GP_CODE' then USE_WT else 0 end),0,0,round(t5.all_outwt/SUM((case when t3.mat_code='GP_CODE' then USE_WT else 0 end))*100,2)) as  CCL"
			", cost_all,round(cost_all/all_outwt,2) as price_unit"
			+ sqlstr_sub +
			+sqlstr_fl +
			" from tcaaia14 t1"
			" left join tsoso02 ts on t1.order_no=ts.SALE_ORDER_SUB_NO"
			" left join"
			" ("
			" select plan_no, prod_Date, sub_backlog_code,SG_SIGN,MAT_THICK,prod_shift_no,prod_shift_group, sum(wt) outwt"
			" from"
			" (select vfree1 as plan_no, stats_period as prod_Date, sub_backlog_code,SG_SIGN,MAT_THICK,prod_shift_no,prod_shift_group, wt from tcaai03"
			" where 1 = 1"
			" and sub_backlog_code = DECODE(@sub_backlog_code,' ',sub_backlog_code,@sub_backlog_code)"
			" and dept_code = @dept_code"
			" and stats_period between @begin_time and @end_time"
			;
		if (flag2 == "1")
		{		
			sqlstr = sqlstr + " union all"
			" select PLAN_NO, prod_Date, sub_backlog_code,SG_SIGN,MAT_THICK,prod_shift_no,prod_shift_group, wt from tcaai08" //加修正的
			" where 1 = 1"
			" AND TYPE_CODE in('U', 'Q')"
			" and sub_backlog_code = DECODE(@sub_backlog_code,' ',sub_backlog_code,@sub_backlog_code)"
			" and dept_code = @dept_code"
			" and prod_Date between @begin_time and @end_time"			
			;
		}
		sqlstr = sqlstr + ")"
			" group by plan_no, prod_Date, sub_backlog_code,SG_SIGN,MAT_THICK,prod_shift_no,prod_shift_group"
			" having sum(wt)!=0"
			" ) t2 on t1.relation_no = t2.plan_no and t1.sub_backlog_code = t2.sub_backlog_code"
			" left join"
			" ("
			" select plan_no, prod_Date, sub_backlog_code, sum(wt) all_outwt"
			" from"
			" (select vfree1 as plan_no, stats_period as prod_Date, sub_backlog_code, wt from tcaai03"
			" where 1 = 1"
			" and sub_backlog_code = DECODE(@sub_backlog_code,' ',sub_backlog_code,@sub_backlog_code)"
			" and dept_code = @dept_code"
			" and stats_period between @begin_time and @end_time"
			;
		if (flag2 == "1")
		{
			sqlstr = sqlstr + " union all"
				" select PLAN_NO, prod_Date, sub_backlog_code, wt from tcaai08" //加修正的
				" where 1 = 1"
				" AND TYPE_CODE in('U', 'Q')"
				" and sub_backlog_code = DECODE(@sub_backlog_code,' ',sub_backlog_code,@sub_backlog_code)"
				" and dept_code = @dept_code"
				" and prod_Date between @begin_time and @end_time"				
				;
		}
		sqlstr = sqlstr + " )"
				" group by plan_no, prod_Date, sub_backlog_code"
				" having sum(wt)!=0"
			" ) t5 on t1.relation_no = t5.plan_no and t1.sub_backlog_code = t5.sub_backlog_code"
			" left join"
			"("
			" select vfree1, stats_period, sub_backlog_code, (case when length(mat_code) >= 20 then 'GP_CODE' else MAT_CODE end) MAT_CODE,SUM(wt) as use_wt,SUM(cost_gj) as cost_gj,SUM(cost_yj)  cost_yj,SUM(cost_cj)  cost_cj"
			" from tcaai04"
			" where 1=1"
			" and sub_backlog_code = DECODE(@sub_backlog_code,' ',sub_backlog_code,@sub_backlog_code)"
			" and dept_code = @dept_code"
			" and stats_period between @begin_time and @end_time"
			" group by vfree1, stats_period, sub_backlog_code,(case when length(mat_code) >= 20 then 'GP_CODE' else MAT_CODE end)"
			" ) t3 on t1.relation_no = t3.vfree1 and t1.sub_backlog_code = t3.sub_backlog_code"
			" left join"
			"(SELECT vfree1,stats_period,sub_backlog_code,sum((case when fl_code = 'PL' then COST_GJ ELSE 0 END )) as cost_PL"
			", sum((case when fl_code = 'A' then COST_GJ ELSE 0 END)) as cost_A"
			", sum((case when fl_code = 'B' then COST_GJ ELSE 0 END)) as cost_B"
			", sum((case when fl_code = 'C' then COST_GJ ELSE 0 END)) as cost_C"
			", sum((case when fl_code = 'D' then COST_GJ ELSE 0 END)) as cost_D"
			", sum((case when fl_code = 'E' then COST_GJ ELSE 0 END)) as cost_E"
			" ,sum(COST_GJ) as cost_all"
			" FROM "
			"( SELECT t04.vfree1, t04.stats_period, t04.sub_backlog_code, (case when length(t04.mat_code) >= 20 then 'PL' ELSE t03.back_code_2 END ) as fl_code,  COST_GJ,COST_CJ, COST_YJ"
			" from tcaai04 t04"
			" LEFT JOIN TCAAC03 t03 on t04.sub_backlog_code = t03.sub_backlog_code and t04.mat_code = t03.mat_code"
			" where 1=1"
			" and t04.sub_backlog_code = DECODE(@sub_backlog_code,' ',t04.sub_backlog_code,@sub_backlog_code)"
			" and t04.dept_code = @dept_code"
			" and t04.stats_period between @begin_time and @end_time"			
			" ) GROUP BY vfree1,stats_period,sub_backlog_code"
			" )t4 on t1.relation_no = t4.vfree1 and t1.sub_backlog_code = t4.sub_backlog_code"
			" where 1=1"
			;
		if (tcaai04["SG_SIGN"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " and sg_sign = @sg_sign ";
		}
		if (tcaai04["VFREE1"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " AND VFREE1=@vfree1 ";
		}
		if (tcaai04["MAT_THICK"].ToDecimal() != 0)
		{
			sqlstr = sqlstr + " AND MAT_THICK=@mat_thick ";
		}
		sqlstr = sqlstr +" and t1.sub_backlog_code = DECODE(@sub_backlog_code,' ',t1.sub_backlog_code,@sub_backlog_code)" 
			" AND relation_no NOT LIKE 'W%'"
			" and t1.dept_code = @dept_code"
			" and t1.stats_period between @begin_time and @end_time"
			" GROUP BY t1.stats_period, t1.relation_no , t1.sub_backlog_code, t1.equ_no"
			", t2.SG_SIGN,t2.MAT_THICK,t2.outwt,t2.prod_shift_no,t2.prod_shift_group" 
			" ,t5.all_outwt"
			",ts.ORD_CUST_CODE,ts.ORD_CUST_NAME"
			",cost_pl,cost_a,cost_b,cost_c,cost_d,cost_e,cost_all"
			;


		////拼接sql		
		//sqlstr = " select t1.vfree1,t1.SUB_BACKLOG_CODE,t1.EQU_NO,t1.PRODUCT_CODE,t1.MAT_THICK,t1.SG_SIGN,t1.STATS_PERIOD as prod_date"
		//	",t1.wt,t1.COST_YJ,decode(t1.wt,0,0,round(t1.COST_YJ/t1.wt,6)) PRICE_UNIT_GJ"
		//	" ,decode(t2.GANGPI_USE,0,0,round(t1.wt/t2.GANGPI_USE*100,2)) as  CCL"
		//	",t3.ZHIZAO_USE,t3.HUISHOU_USE,t3.DONGLI_USE,t3.GONGZI_USE,t3.ZHAGUN_USE"
		//	" ,t2.*"
		//	" from "
		//	//产量
		//	" (select vfree1,SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,sum(WT) as WT,SUM(COST_YJ) COST_YJ"
		//	" from tcaai03"
		//	" where 1=1"
		//	" AND SUB_BACKLOG_CODE = decode(@sub_backlog_code,' ',SUB_BACKLOG_CODE,@sub_backlog_code)  "
		//	+ sqlstr_where +
		//	" group by vfree1,SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD) t1"
		//	" left join " //明细项消耗
		//	" (select vfree1,SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD,sum(case when length(mat_code)>=20 then WT else 0 end) GANGPI_USE"
		//	+ sqlstr_sub +
		//	" from tcaai04"
		//	" where 1=1"
		//	" AND SUB_BACKLOG_CODE = decode(@sub_backlog_code,' ',SUB_BACKLOG_CODE,@sub_backlog_code)  "
		//	+ sqlstr_where +
		//	" group by vfree1,SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD) t2 "
		//	" on t1.vfree1 = t2.vfree1 AND t1.SUB_BACKLOG_CODE =  t2.SUB_BACKLOG_CODE and t1.EQU_NO = t2.EQU_NO and t1.PRODUCT_CODE=t2.PRODUCT_CODE"
		//	" AND t1.MAT_THICK =  t2.MAT_THICK AND t1.SG_SIGN =  t2.SG_SIGN and t1.STATS_PERIOD=t2.stats_period"
		//	" left join " //制造费用
		//	" (select vfree1,t.SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD"
		//	",sum(case when BACK_CODE_2 ='C' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='C' and @flag !='1' then COST_YJ else 0 end) ZHIZAO_USE"
		//	" ,sum(case when BACK_CODE_2 ='D' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='D' and @flag !='1' then COST_YJ  else 0 end) HUISHOU_USE"
		//	" ,sum(case when BACK_CODE_2 ='A' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='A' and @flag !='1' then COST_YJ  else 0 end) DONGLI_USE"
		//	" ,sum(case when BACK_CODE_2 ='B' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='B' and @flag !='1' then COST_YJ  else 0 end) GONGZI_USE"
		//	" ,sum(case when BACK_CODE_2 ='E' and @flag ='1' and OUTPUT!=0 then round(COST_YJ/OUTPUT,6) when BACK_CODE_2 ='E' and @flag !='1' then COST_YJ  else 0 end) ZHAGUN_USE"
		//	" from tcaai04 t"
		//	" left join tcaac03  t03 on t.mat_code = t03.mat_code and t.sub_backlog_code = t03.sub_backlog_code"
		//	" where 1=1"
		//	" AND t.SUB_BACKLOG_CODE = decode(@sub_backlog_code,' ',t.SUB_BACKLOG_CODE,@sub_backlog_code) "
		//	+ sqlstr_where +
		//	" group by vfree1,t.SUB_BACKLOG_CODE,EQU_NO,PRODUCT_CODE,MAT_THICK,SG_SIGN,STATS_PERIOD) t3 "
		//	" on t1.vfree1 = t3.vfree1 AND t1.SUB_BACKLOG_CODE =  t3.SUB_BACKLOG_CODE and t1.EQU_NO = t3.EQU_NO and t1.PRODUCT_CODE=t3.PRODUCT_CODE"
		//	" AND t1.MAT_THICK =  t3.MAT_THICK AND t1.SG_SIGN =  t3.SG_SIGN and t1.STATS_PERIOD=t3.stats_period "
		//	;
		Log::Trace("", __FUNCTION__, "sqlstr=[{0}]  ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("sg_sign", tcaai04["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaai04["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai04["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("vfree1", tcaai04["VFREE1"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai04["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("flag", flag);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

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

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}